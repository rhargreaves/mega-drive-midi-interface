FROM rhargreaves/gendev:circle-7
RUN apt-get -y update && \
	apt-get -y install \
		build-essential \
		cmake
