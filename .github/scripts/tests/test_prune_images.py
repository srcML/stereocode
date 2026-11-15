"""Offline tests of the deletion of the images that lost their tag in the Docker workflow."""

import email.message
import importlib.util
import io
import os
import unittest
import urllib.error
from pathlib import Path
from unittest import mock

SCRIPT = Path(__file__).resolve().parents[1] / "prune_images.py"
SPEC = importlib.util.spec_from_file_location("prune_images", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
prune = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(prune)

PACKAGES = "https://api.github.com/orgs/srcml/packages/container"
VERSION_URL = f"{PACKAGES}/stereocode/versions/1"


def version(number, name, tags=()):
    return {"id": number, "name": name, "metadata": {"container": {"tags": list(tags)}}}


class KeptTagsTest(unittest.TestCase):
    def test_latest_and_the_tags_of_existing_releases_keep_an_image(self):
        releases = {"2.0.0"}
        self.assertTrue(prune.kept(["latest"], releases))
        self.assertTrue(prune.kept(["2.0.0"], releases))

    def test_a_removed_release_a_tag_of_a_srcml_release_and_no_tag_keep_no_image(self):
        releases = {"2.0.0"}
        self.assertFalse(prune.kept(["1.0.0"], releases))
        self.assertFalse(prune.kept(["2.0.0-srcml1.2.0"], releases))
        self.assertFalse(prune.kept(["build-amd64"], releases))
        self.assertFalse(prune.kept([], releases))


class VersionsToDeleteTest(unittest.TestCase):
    def test_the_images_of_a_kept_index_stay(self):
        versions = [version(1, "sha256:index", ["2.0.0", "latest"]), version(2, "sha256:amd64"), version(3, "sha256:arm64"),
                    version(4, "sha256:old-index"), version(5, "sha256:build", ["build-amd64"])]
        needed = prune.listed_digests({"manifests": [{"digest": "sha256:amd64"}, {"digest": "sha256:arm64"}]})
        self.assertEqual([v["id"] for v in prune.to_delete(versions, {"2.0.0"}, needed)], [4, 5])

    def test_a_single_image_lists_no_digest(self):
        self.assertEqual(prune.listed_digests({"config": {}, "layers": []}), set())


class DeleteTest(unittest.TestCase):
    def test_a_version_that_is_gone_counts_as_deleted_elsewhere(self):
        gone = urllib.error.HTTPError(VERSION_URL, 404, "Not Found", email.message.Message(), io.BytesIO())
        with mock.patch.object(prune.urllib.request, "urlopen", side_effect=gone):
            self.assertFalse(prune.delete(VERSION_URL, {}))

    def test_any_other_error_stops_the_deletion(self):
        denied = urllib.error.HTTPError(VERSION_URL, 403, "Forbidden", email.message.Message(), io.BytesIO())
        with mock.patch.object(prune.urllib.request, "urlopen", side_effect=denied), self.assertRaises(urllib.error.HTTPError):
            prune.delete(VERSION_URL, {})


class MainTest(unittest.TestCase):
    def run_offline(self, published):
        """Run main() on the package without network access and return the deleted URLs and the GET requests."""
        indexes = {"2.0.0": {"manifests": [{"digest": "sha256:amd64"}, {"digest": "sha256:arm64"}]}}
        requests = []

        def fetch(url, headers):
            requests.append((url, headers))
            if url.startswith(f"{PACKAGES}/stereocode/versions"):
                return published
            if "/repos/srcML/stereocode/tags" in url:
                return [{"name": "v2.0.0"}]
            if url.startswith("https://ghcr.io/token"):
                return {"token": "pull"}
            return indexes.get(url.rsplit("/", 1)[1], {"config": {}, "layers": []})

        deleted = []

        def urlopen(request):
            deleted.append(request.full_url)
            return io.BytesIO()

        environment = {"GITHUB_TOKEN": "offline", "GITHUB_ACTOR": "someone", "GITHUB_REPOSITORY": "srcML/stereocode",
                       "IMAGE": "ghcr.io/srcml/stereocode"}
        with mock.patch.dict(os.environ, environment), mock.patch.object(prune, "fetch", fetch), \
                mock.patch.object(prune.urllib.request, "urlopen", urlopen), mock.patch("builtins.print"):
            prune.main()
        return deleted, requests

    def test_the_package_loses_the_images_that_no_kept_tag_needs(self):
        published = [version(1, "sha256:index", ["2.0.0", "latest"]), version(2, "sha256:amd64"), version(3, "sha256:arm64"),
                     version(4, "sha256:old-index"), version(5, "sha256:old-amd64"), version(6, "sha256:build", ["build-arm64"])]
        deleted, _ = self.run_offline(published)
        self.assertEqual(deleted, [f"{PACKAGES}/stereocode/versions/4", f"{PACKAGES}/stereocode/versions/5",
                                   f"{PACKAGES}/stereocode/versions/6"])

    def test_the_registry_takes_the_token_of_the_workflow_for_a_private_package(self):
        _, requests = self.run_offline([version(1, "sha256:index", ["2.0.0"])])
        token_requests = [headers for url, headers in requests if url.startswith("https://ghcr.io/token")]
        self.assertEqual(len(token_requests), 1)
        self.assertTrue(token_requests[0]["Authorization"].startswith("Basic "))


if __name__ == "__main__":
    unittest.main()
