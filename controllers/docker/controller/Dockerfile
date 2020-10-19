# Debian slim with nodejs

######## root image
FROM debian:latest as root

# run as root user for simplicity
USER root

# unattended apt installs
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
	apt-get install -y wget procps xz-utils

######## Install application
FROM root as application

# add controller source code to build
RUN mkdir -p /tmp/build
COPY controller.tgz /tmp/build

RUN apt-get update && \
    apt-get install -y build-essential

## touch a file so we can see which files were installed after this point
RUN /usr/bin/touch /tmp/__install_start

RUN apt-get install -y libmosquitto-dev libjsoncpp-dev libpqxx-dev libpq-dev

RUN cd /tmp/build && \
	tar xvfz controller.tgz && \
	make && make install

RUN mkdir -p /tmp/__saved_changes && \
	tar cf - $(find -L /usr /tmp -type f -cnewer /tmp/__install_start) | tar xf - -C /tmp/__saved_changes

######## Install dependencies
## The following dependencies are required and were installed in the application layer
## through build-essentials.  To minimize the final image, we throw away build-essentials
## from the application layer and reinstall the required dependencies
FROM root as dependencies

## touch a file so we can see which files were installed after this point
RUN /usr/bin/touch /tmp/__install_start

RUN apt-get update && \
	apt-get install -y libldap-2.4-2 libkeyutils1

RUN mkdir -p /tmp/__saved_changes && \
	tar cf - $(find -L /usr /lib -type f -cnewer /tmp/__install_start) | tar xf - -C /tmp/__saved_changes

######## Final Image
FROM root as final

# copy saved changes in /usr and /lib from all layers
COPY --from=application /tmp/__saved_changes /
COPY --from=dependencies /tmp/__saved_changes /

COPY start-app /
COPY entrypoint /

# install supervisor
RUN apt-get install -y supervisor
COPY supervisord.conf /etc/supervisor/conf.d/supervisord.conf

ENTRYPOINT ["/entrypoint"]
CMD ["/start-app"]
