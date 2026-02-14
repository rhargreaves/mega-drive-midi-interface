ARG BASE_IMAGE_SUFFIX=-mw-bs
FROM ghcr.io/rhargreaves/docker-sgdk:v2.00${BASE_IMAGE_SUFFIX}
USER root
RUN apt-get -y update && \
    apt-get -y install \
    build-essential \
    git \
    cmake \
    gdb \
    gdbserver \
    wget \
    unzip && \
    git config --global --add safe.directory "*"
WORKDIR /app
COPY . /app
EXPOSE 2345
ENTRYPOINT []
