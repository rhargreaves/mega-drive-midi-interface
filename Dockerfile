FROM registry.gitlab.com/doragasu/docker-sgdk:v1.70
USER root
RUN apt-get -y update && \
    apt-get -y install \
    build-essential \
    cmake \
    gdb \
    gdbserver \
    valgrind
# USER m68k
WORKDIR /app
COPY tests/cmocka* /app/tests/
COPY tests/Makefile /app/tests/Makefile
RUN make -C tests cmocka
COPY . /app
EXPOSE 2345
ENTRYPOINT []
