FROM ubuntu:latest

WORKDIR /data

COPY mysh .

RUN chmod +x mysh

ENTRYPOINT ["./mysh"]