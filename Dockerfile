# syntax=docker/dockerfile:1
# Stereocode and srcML in one image. Each build takes the newest srcML release, unless SRCML_VERSION names one,
# the newest Ubuntu packages, and the source of this folder, and the tests of Stereocode run before the final stage.

ARG UBUNTU=24.04

FROM ubuntu:${UBUNTU} AS srcml
ARG UBUNTU
ARG TARGETARCH
ARG SRCML_VERSION=latest
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends ca-certificates curl jq \
 && rm -rf /var/lib/apt/lists/*
COPY docker/fetch-srcml.sh /usr/local/bin/
RUN --mount=type=secret,id=github_auth fetch-srcml.sh "$SRCML_VERSION" "$UBUNTU" "$TARGETARCH" /srcml

FROM ubuntu:${UBUNTU} AS build
ARG DEBIAN_FRONTEND=noninteractive
RUN --mount=type=bind,from=srcml,source=/srcml,target=/tmp/srcml \
    apt-get update \
 && apt-get install -y --no-install-recommends cmake ninja-build g++ libxml2-dev /tmp/srcml/*.deb \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /stereocode
COPY . .
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSTEREOCODE_REQUIRE_SRCML=ON \
 && cmake --build build \
 && ctest --test-dir build -j "$(nproc)" --output-on-failure

FROM ubuntu:${UBUNTU}
ARG DEBIAN_FRONTEND=noninteractive
RUN --mount=type=bind,from=srcml,source=/srcml,target=/tmp/srcml \
    apt-get update \
 && apt-get install -y --no-install-recommends /tmp/srcml/srcml_*.deb \
 && rm -rf /var/lib/apt/lists/*
COPY --from=build /stereocode/build/stereocode /usr/local/bin/
COPY docker/entrypoint.sh /usr/local/bin/
WORKDIR /data
LABEL org.opencontainers.image.source="https://github.com/srcML/stereocode" \
      org.opencontainers.image.description="Stereocode and srcML, which classify the methods and types of C, C++, C#, and Java code by their stereotypes" \
      org.opencontainers.image.licenses="GPL-3.0-only"
ENTRYPOINT ["entrypoint.sh"]
CMD ["stereocode", "--help"]
