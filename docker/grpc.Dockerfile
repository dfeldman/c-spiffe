FROM ubuntu:focal

LABEL maintainer="Willian Alves <wra@cesar.org.br>"

ARG GRPC_VERSION=1.34.0
ARG NUM_JOBS=8

# Install Spire Server
ARG SPIRE_VERSION=0.12.0
ARG SPIRE_RELEASE=https://github.com/spiffe/spire/releases/download/v${SPIRE_VERSION}/spire-${SPIRE_VERSION}-linux-x86_64-glibc.tar.gz
ARG SPIRE_DIR=/opt/spire

ENV DEBIAN_FRONTEND noninteractive

# Install package dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        software-properties-common \
        autoconf \
        automake \
        libtool \
        pkg-config \
        ca-certificates \
        wget \
        git \
        curl \
        tar \
        gzip \
        vim \
        gdb \
        valgrind \
        cmake \
        libssl-dev \
        liburiparser1 liburiparser-dev \
        protobuf-compiler \
        libprotobuf-dev \
        check \
        lcov \
        gcovr \
	libjansson-dev \
        libcjose-dev \
        libgtest-dev \
        libgmock-dev
RUN apt-get clean

# gRPC
# https://github.com/grpc/grpc/tree/master/src/cpp
# https://github.com/grpc/grpc/blob/master/BUILDING.md

# RUN apt-get install -y build-essential autoconf libtool pkg-config && \
RUN cd /tmp && git clone --recurse-submodules -b v${GRPC_VERSION} https://github.com/grpc/grpc
RUN mkdir -p /tmp/grpc/cmake/build
RUN cd /tmp/grpc/cmake/build && cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=ON ../..
RUN sed -i '7,13d' /tmp/grpc/third_party/benchmark/test/cxx03_test.cc 
RUN cd /tmp/grpc/cmake/build && make -j${NUM_JOBS} && make install

RUN curl --silent --location $SPIRE_RELEASE | tar -xzf -
RUN mv spire-${SPIRE_VERSION} ${SPIRE_DIR}

RUN ln -s /opt/spire/bin/spire-server /usr/bin/spire-server
RUN ln -s /opt/spire/bin/spire-agent /usr/bin/spire-agent

# Configure Spire
WORKDIR ${SPIRE_DIR}
COPY docker/init-server.sh .
RUN ls ${SPIRE_DIR}
RUN chmod +x init-server.sh

