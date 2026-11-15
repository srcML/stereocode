"""Deletes the images of the Docker workflow on GitHub that no tag of a release or of the newest build needs.

An image of several architectures lists the image of each architecture, which carries no tag of its own and stays with it."""

import base64
import json
import os
import re
import urllib.error
import urllib.request
from typing import Any

INDEX_TYPES = ("application/vnd.oci.image.index.v1+json, application/vnd.docker.distribution.manifest.list.v2+json, "
               "application/vnd.oci.image.manifest.v1+json, application/vnd.docker.distribution.manifest.v2+json")
RELEASE_TAG = re.compile(r"^\d+\.\d+\.\d+$")


def kept(tags: list[str], releases: set[str]) -> bool:
    """Tell whether its tags keep an image, namely latest or the tag of a release that exists, such as 2.0.0."""
    return any(tag == "latest" or (RELEASE_TAG.match(tag) is not None and tag in releases) for tag in tags)


def to_delete(versions: list[dict], releases: set[str], needed: set[str]) -> list[dict]:
    """Return the versions that their tags do not keep and whose digest no kept image lists."""
    return [v for v in versions if not kept(v["metadata"]["container"]["tags"], releases) and v["name"] not in needed]


def listed_digests(manifest: dict) -> set[str]:
    """Return the digests of the images that an image index lists, or none for a single image."""
    return {entry["digest"] for entry in manifest.get("manifests", [])}


def fetch(url: str, headers: dict[str, str]) -> Any:
    """Return the JSON answer of a GET request."""
    with urllib.request.urlopen(urllib.request.Request(url, headers=headers)) as reply:
        return json.load(reply)


def fetch_all(url: str, headers: dict[str, str]) -> list:
    """Return every item of a list of the GitHub API, which gives at most 100 items per page."""
    items, page = [], 1
    while True:
        batch = fetch(f"{url}?per_page=100&page={page}", headers)
        items += batch
        if len(batch) < 100:
            return items
        page += 1


def delete(url: str, headers: dict[str, str]) -> bool:
    """Delete one version and return False when it is already gone."""
    try:
        urllib.request.urlopen(urllib.request.Request(url, method="DELETE", headers=headers)).close()
    except urllib.error.HTTPError as error:
        if error.code != 404:
            raise
        return False
    return True


def prune(image: str, releases: set[str], github: dict[str, str], login: str) -> None:
    """Delete the versions of one package, such as ghcr.io/srcml/stereocode, that no kept image needs, and print each deleted digest."""
    _, owner, package = image.split("/")
    api = f"https://api.github.com/orgs/{owner}/packages/container/{package}/versions"
    versions = fetch_all(api, github)
    # A package is private until an owner makes it public, therefore the registry takes the token of the workflow
    pull = fetch(f"https://ghcr.io/token?service=ghcr.io&scope=repository:{owner}/{package}:pull", {"Authorization": f"Basic {login}"})["token"]
    registry = {"Authorization": f"Bearer {pull}", "Accept": INDEX_TYPES}
    needed = set()
    for version in versions:
        tags = version["metadata"]["container"]["tags"]
        if kept(tags, releases):
            for tag in tags:
                needed |= listed_digests(fetch(f"https://ghcr.io/v2/{owner}/{package}/manifests/{tag}", registry))
    for version in to_delete(versions, releases, needed):
        if delete(f"{api}/{version['id']}", github):
            print(f"deleted {package} {version['name']}")


def main() -> None:
    """Prune the package that the Docker workflow names in IMAGE."""
    token = os.environ["GITHUB_TOKEN"]
    github = {"Authorization": f"Bearer {token}", "Accept": "application/vnd.github+json"}
    login = base64.b64encode(f"{os.environ['GITHUB_ACTOR']}:{token}".encode()).decode()
    tags = fetch_all(f"https://api.github.com/repos/{os.environ['GITHUB_REPOSITORY']}/tags", github)
    releases = {tag["name"].removeprefix("v") for tag in tags}
    prune(os.environ["IMAGE"], releases, github, login)


if __name__ == "__main__":
    main()
