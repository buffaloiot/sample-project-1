# Debian slim with nodejs

### root image
FROM debian:stretch-slim as root

# run as root user for simplicity
USER root

# unattended apt installs
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
	apt-get install -y wget procps xz-utils

### NodeJS image
FROM root as nodejs

RUN	wget https://nodejs.org/dist/v14.9.0/node-v14.9.0-linux-x64.tar.xz -O /tmp/nodejs.tar.xz && \
	tar -C /usr --strip-components 1 -xJf /tmp/nodejs.tar.xz && \
	rm /tmp/nodejs.tar.xz && \
	mkdir /usr/etc && \
	echo "prefix=/usr" >> /usr/etc/npmrc

### Install application
FROM nodejs as application

COPY device.tgz /tmp

# Install application from tarball using npm pack.
RUN npm install -g /tmp/device.tgz && \
	rm -r /tmp/device.tgz

### Final Image
FROM application as final

COPY start-app /
COPY entrypoint /

# install supervisor
RUN npm install -g supervisor@0.12.0

ENTRYPOINT ["/entrypoint"]
CMD ["/start-app"]
