FROM debian:bookworm

ARG DEBIAN_FRONTEND=noninteractive

ENV IN_DOCKER_CONTAINER=1

RUN apt-get update
RUN apt-get install -y python3 python3-pip default-jre

COPY src/python src/python

RUN apt-get install -y python3-numpy
RUN pip install --break-system-packages --no-cache-dir orjson simplejson posix-ipc
RUN pip install --break-system-packages src/python

WORKDIR /app