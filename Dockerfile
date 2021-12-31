FROM rhargreaves/gendev:circle-21
RUN apt-get -y update && \
	apt-get -y install \
	build-essential \
	cmake \
	gdb \
	gdbserver \
	valgrind
WORKDIR /app
COPY tests/cmocka* /app/tests/
COPY tests/Makefile /app/tests/Makefile
RUN make -C tests cmocka
COPY . /app
EXPOSE 2345
ENTRYPOINT []
