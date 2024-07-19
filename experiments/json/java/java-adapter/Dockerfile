FROM debian:latest

ENV IN_DOCKER_CONTAINER=1

RUN apt-get update -y
RUN apt-get install -y make default-jdk maven

WORKDIR /app

COPY pom.xml .
COPY Makefile.java-adapter Makefile