# 使用包含部分必要依赖ubuntu的基础镜像
FROM ubuntu:latest

# 安装gcc和readline库
RUN apt-get update && \
    apt-get install -y gcc libreadline-dev make

# 将源码和编译好的特定二进制文件复制到/src
COPY cyshell.c /src/cyshell.c
COPY ping /src/ping
COPY traceroute /src/traceroute

# 设置工作目录为/src
WORKDIR /src

# 编译CyShell
RUN gcc cyshell.c -std=gnu99 -lreadline -o cyshell

# 设置入口点
ENTRYPOINT ["/src/cyshell"]