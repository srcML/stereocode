#!/bin/sh
# Downloads the Ubuntu packages of srcML for one architecture from a GitHub release of srcML
# and checks each package against the SHA-256 digest that GitHub gives for it.
# Usage: fetch-srcml.sh <srcML version or latest> <Ubuntu version> <architecture> <folder>
set -eu

version=$1
ubuntu=$2
arch=$3
folder=$4

release=latest
[ "$version" = latest ] || release="tags/v$version"

# The build secret github_auth holds an authorization header that raises the rate limit of the GitHub API
auth=/run/secrets/github_auth
[ -s "$auth" ] || auth=/dev/null

mkdir -p "$folder"
curl -fsSL -H @"$auth" "https://api.github.com/repos/srcML/srcML/releases/$release" |
    jq -r --arg pattern "^srcml(-dev)?_.*_ubuntu${ubuntu}_${arch}[.]deb$" \
        '.assets[] | select(.name | test($pattern)) | "\(.digest) \(.name) \(.browser_download_url)"' > "$folder/assets.txt"

if [ "$(wc -l < "$folder/assets.txt")" -ne 2 ]; then
    echo "The srcML release $version has no two Ubuntu $ubuntu packages for $arch" >&2
    exit 1
fi

while read -r digest name url; do
    curl -fsSL -o "$folder/$name" "$url"
    echo "${digest#sha256:}  $folder/$name" | sha256sum -c -
done < "$folder/assets.txt"
rm "$folder/assets.txt"
