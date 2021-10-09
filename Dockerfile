FROM rhargreaves/gendev:circle-21
RUN apt-get -y update && \
	apt-get -y install \
	build-essential \
	cmake \
	gdb \
	gdbserver
WORKDIR /app
COPY . /app
EXPOSE 2345
ENTRYPOINT []
