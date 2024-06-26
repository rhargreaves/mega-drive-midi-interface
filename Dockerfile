FROM ghcr.io/rhargreaves/docker-sgdk:v1.70-mw
USER root
RUN apt-get -y update && \
    apt-get -y install \
    build-essential \
    cmake \
    gdb \
    gdbserver \
    valgrind \
    wget \
    unzip
WORKDIR /app
COPY tests/cmocka-*.tar.xz /app/tests/
COPY tests/Makefile /app/tests/Makefile
RUN make -C tests cmocka
COPY . /app
EXPOSE 2345
ENTRYPOINT []
