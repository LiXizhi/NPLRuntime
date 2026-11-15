FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        ca-certificates \
        libboost-all-dev \
        libssl-dev \
        libcurl4-openssl-dev \
        zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/NPLRuntime
COPY . /opt/NPLRuntime

ARG JOBS=""
ARG CMAKE_BUILD_TYPE="Release"

RUN JOBS="${JOBS}" ./build_linux.sh server "${CMAKE_BUILD_TYPE}" --install \
    && rm -rf /opt/NPLRuntime/build-server

ENTRYPOINT ["npl"]
CMD ["NPLRuntime/tests/helloworld.lua"]
