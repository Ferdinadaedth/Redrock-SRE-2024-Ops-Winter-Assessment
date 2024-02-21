# CyShell (cysh)

CyShell是一个用C实现的简单类Unix命令行解释器（shell），支持基本的命令行操作。

**注意：Cyshell只是我为了重庆邮电大学红岩网校工作站2024年寒假考核作业而编写的实验性质的Shell，目前尚且不能作为完整的Shell来使用，切勿用于生产环境！**

## 功能（带有*的项目为作业要求之外的功能）

- **内建命令**：CyShell目前支持下列内建命令：
  
  - `cd`：更改当前工作目录。
  - `pwd`：显示当前工作目录的完整路径。
  - `help`：显示帮助信息，包括所有内建命令的列表。
  - `exit`：退出CyShell。
  - `echo`：显示一行文本。
  - `mkdir`：创建新目录。

- **程序执行**：用户可以执行任何在PATH变量和当前工作目录下的程序或脚本。

- **管道**：CyShell支持简单的管道操作，允许将一个命令的输出作为另一个命令的输入。

- **重定向**：CyShell支持基本的输入输出重定向。

- ***信号处理**：CyShell可以处理 `SIGINT` 和 `SIGTSTP` 信号，允许用户使用快捷键如Ctrl+C和Ctrl+Z来进行控制。

- ***动态内存管理**：CyShell可以动态地调整其缓冲区大小以适应不同长度的输入。

- ***命令历史记录**：CyShell使用readline库来保存命令历史，并且支持行编辑功能。

## 开始使用

### 从Docker Hub获取

```bash
docker pull cysnies/cyshell:latest
docker run -it --rm cysnies/cyshell:latest
```

### 从Dockerfile构建

```bash
docker build -t cyshell .
docker run -it --rm cyshell
```

### 编译

### Cyshell程序本体

1. 确保你的系统中安装了gcc编译器、readline库。

2. 使用gcc编译CyShell：

```bash
gcc main.c -lreadline -o cyshell
```

### 附加的ping和traceroute程序

1. 确保你的系统中安装了rust工具链（rustc和cargo）

2. 定位到相应的程序目录下

```bash
cargo build --release
```

3. 将编译好的程序从target/release目录下复制到cyshell所在目录下

## 在Cyshell中使用附加的ping和traceroute程序（作业要求）

**注意：因为涉及到底层套接字调用，以非docker方式运行这两个程序时需要root权限**

```cysh
./ping [目标地址（仅支持ipv4）]
./traceroute [目标地址（仅支持ipv4）]
```

## 已知问题

1. 目前的管道实现不支持连续的管道操作（例如`cmd1 | cmd2 | cmd3`）。

2. 重定向功能仅限于基本输入和输出重定向，尚未支持错误输出重定向等。

3. 目前只能处理含单层引号的命令，尚未实现对多层嵌套引号的解析。

4. 对某些特殊字符的解析可能存在问题（待验证）。

5. 在解析命令时可能存在潜在的内存安全问题（待验证）。 

## 关于ping和traceroute命令实现的说明

根据作业要求，需要在shell中实现内建的ping和traceroute命令。但我在尝试用C实现ping和traceroute的过程中发现直接使用原始套接字编程过于繁琐，并且这两个命令本身就不适合作为shell内建命令实现，最后使用rust的pnet库独立实现了ping和traceroute的基本功能，将其作为Cyshell的附加程序。使用时直接在Cyshell中运行编译好的二进制文件即可。

## 关于代码编写过程中AI工具使用的说明

我在程序编写过程中，在遵循适度原则的前提下以多种形式使用了多种AI工具（包括但不限于ChatGPT、Bing Copilot、Google Gemini、GIthub Copilot）。大部分代码仍是我本人编写或修改而来的，没有照搬AI生成的代码。

AI工具的在本次开发中的主要用途如下：

- 基于问答方式协助理解代码

- 部分注释的补充编写

- 程序健壮性优化（协助发现和解决诸如内存溢出、空指针等错误以及一些潜在问题）

- 变量名、参数名、函数名以及注释的规范化和代码风格统一化

- 提示部分功能的实现思路，比如对含重定向和管道符命令的高效率切分方法等

- 辅助查找相关文献资料、库函数API文档等

## 关于代码参考和文献资料使用的说明

在编写这个Shell程序之前我对Shell只有一点粗浅的了解，所以在程序编写过程中大量阅读了各类文章，包括但不限于博客园、CSDN上一些大佬和前辈的教学文章以及一些相关书籍，同时也参考了主流Shell（如Bash、Zsh）的实现方式。

**部分参考资料如下：**

《Unix/Linux编程实践教程》

《Linux/Unix系统编程手册》

https://www.zhihu.com/question/28606292
[https://zhuanlan.zhihu.com/p/361557033]()

https://zhuanlan.zhihu.com/p/360923356

[https://blog.csdn.net/dxyt2002/article/details/129800496](https://blog.csdn.net/dxyt2002/article/details/129800496)

https://brennan.io/2015/01/16/write-a-shell-in-c/

[Build a Linux Shell [Part I] | HackerNoon](https://hackernoon.com/lets-build-a-linux-shell-part-i-bz3n3vg1)

[Making your own Linux Shell in C - GeeksforGeeks](https://www.geeksforgeeks.org/making-linux-shell-c/)

[https://fasionchan.com/network/icmp/ping-c/]( https://fasionchan.com/network/icmp/ping-c/)
