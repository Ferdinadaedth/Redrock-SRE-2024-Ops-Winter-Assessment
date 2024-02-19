FROM gcc:latest

RUN  mkdir /usr/src/mysh

COPY . /usr/src/mysh

WORKDIR /usr/src/mysh

RUN  gcc shell.c 

CMD ["./a.out"]
