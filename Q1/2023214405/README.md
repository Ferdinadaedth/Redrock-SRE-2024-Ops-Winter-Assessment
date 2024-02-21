

# level0

## 简介

这是一个简单的控制容器的Python脚本

## 详情

通过Docker SDK for Python实现了创建容器、启动容器、停止容器、重启容器和销毁容器等操作

## 使用方式

命令

```
python controller.py
```

## 参考文献

[Containers — Docker SDK for Python 7.0.0 documentation (docker-py.readthedocs.io)](https://docker-py.readthedocs.io/en/stable/containers.html#container-objects)

[docker-py(Docker SDK for Python)使用指南-CSDN博客](https://blog.csdn.net/qq_42730750/article/details/128903132?ops_request_misc=%7B%22request%5Fid%22%3A%22170772919816800182753296%22%2C%22scm%22%3A%2220140713.130102334..%22%7D&request_id=170772919816800182753296&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduend~default-2-128903132-null-null.142^v99^pc_search_result_base4&utm_term=Docker SDK&spm=1018.2226.3001.4187)



# level1

## 简介

这是一个简单的注册登录后端web项目

## 详情

使用docker-compose创建了三个容器

nginx：运行nginx提供前端网页服务

python容器：运行编写的脚本，处理前端提交的数据，并将其储存到mariadb数据库中

mariadb容器：运行mariadb数据库，将容器内的数据库挂载到当前目录，实现数据持久化

## 使用方式

首先修改`index.html`与`register.html`，将`127.0.0.1`修改为本机ip(也可以不改)

命令

```
python docker-compose.py
```

待所有容器均开始运行后，浏览器打开http://127.0.0.1即可进入登入页面

## 参考文献

[Services top-level elements | Docker Docs](https://docs.docker.com/compose/compose-file/05-services/)

[12.2. 使用 python 优雅地编排 Docker 容器 (netkiller.cn)](https://www.netkiller.cn/container/container/netkiller.docker.html)



# level2

## 简介

这是一个体现了负载均衡的项目，运行脚本可以选择部署的前端和后端服务器的数量，并自动配置负载均衡规则

## 详情

`test`文件夹内的前端和后端文件经过标记，在容器运行后可以更加直观的体现均衡负载。

`end`文件夹中的程序更注重实际情况，加入了自动编写`nginx.conf`的程序。使脚本更加自动化

## 参考文献

[使用docker搭建nginx集群,实现负载均衡 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/157183707?utm_id=0)

# level3

未完成

监控集群中容器实例的CPU使用率或请求处理时间之类的好像要使用k8s，docker自带的Swarms好像实现不了，但k8s没时间看了/(ㄒoㄒ)/~~



# level4

## 简介

这是一个检查容器健康状态的脚本

## 详情

容器的健康状态是可以在``docker inspect {容器id}`打印出容器详细信息中找到的，这个命令在[Docker SDK for Python](https://docker-py.readthedocs.io/en/stable/index.html)中对应的是`container.attrs`。

`test.py`是用于在编写`check_health`函数之前，查看`health`值位置的工具。

最后用`controller.py`在后台调用`healthcheck.py`，并在结束本进程之前停止`healthcheck.py`的进程。一开始打算使用subprocess库，但是好像因为没有处理好输入输出，导致运行不了，最终使用os库实现了，但是偶尔会没有杀死`healthcheck.py`的进程，需要手动`kill`一下。

## 使用方式

命令

```
python controller.py
```

当然也可以只调用`healthcheck.py`

```
python healthcheck.py
```

