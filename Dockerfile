FROM rhargreaves/gendev:circle-10
RUN apt-get -y update && \
	apt-get -y install \
	build-essential \
	cmake
