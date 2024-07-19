FROM debian:bookworm

ENV IN_DOCKER_CONTAINER=1

RUN apt-get update -y
RUN apt-get install -y make default-jdk maven

ENV JAVA_HOME /usr/lib/jvm/java-17-openjdk-amd64/
RUN export JAVA_HOME

WORKDIR /app

COPY pom.xml .
COPY Makefile.java-adapter Makefile