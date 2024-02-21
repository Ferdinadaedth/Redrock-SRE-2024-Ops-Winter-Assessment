from support import *
import docker
import time
import os
import signal

client = docker.from_env()
pid = os.getppid()
while True:
    try:
        os.kill(pid, signal.SIGIO)
        print(f"进程 {pid} ")
    except OSError:
        print(f"进程 {pid} 已经结束")
        break
    containers = client.containers.list()
    for container in containers:
        health = check_health(container)
        if health == 'unhealthy':
            print(f"容器{container.name}发现异常\n容器的ID为：{container.short_id}\n容器的镜像为：{container.image}\n容器的状态为：{container.status}")
            container.restart()
            print(f'已重启容器 {container.name}\n')
#    print("容器健康检查后台运行中")
    time.sleep(5)
