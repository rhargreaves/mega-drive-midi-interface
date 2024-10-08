FROM ghcr.io/rhargreaves/docker-sgdk:v2.00-mw-bs
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
COPY tests/cmocka-*.tar.xz /app/tests/
COPY tests/Makefile /app/tests/Makefile
RUN make -C tests cmocka
COPY . /app
EXPOSE 2345
ENTRYPOINT []
