FROM ubuntu:latest
COPY hshshell.c /hshdshell
RUN apt-get update && \
    apt-get install -y gcc
WORKDIR /hshdshell
RUN gcc hshshell.c -o hshshell
