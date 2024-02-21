# 导入所需的模块
import docker
import time
import os

client = docker.from_env()

#列出所有镜像
def list_images():
    images = client.images.list()
    print(f"\tTAG\t\t\tID")
    for image in images:
        print(f"{image.tags}\t{image.short_id}")

# 检查镜像
def test_image(image):
    try:
        client.images.get(image)
        return 0
    except:
        print("镜像不存在")
        return 1

#列出所有容器
def list_container():
    containers = client.containers.list(all=True)
    print("CONTAINER ID\t\tIMAGE\t\t\t\tSTATUS\t\tNAMES")

    for container in containers:
        print(f"{container.short_id}\t{container.image}\t\t{container.status}\t\t{container.name}")

# 检查容器
def test_container(id):
    try:
        client.containers.get(id)
        return 1
    except:
        print("容器不存在")
        return 0

# 创建容器
def create_container(image, name):

    container = client.containers.create(image, name=name)
    print(f"已创建容器 {name}")
    print(f"容器的名称为：{container.name}\n容器的ID为：{container.short_id}\n容器的镜像为：{container.image}\n容器的状态为：{container.status}")


# 启动容器
def start_container(id):
    container = client.containers.get(id)
    if container.status == "running":
        print("容器已处于运行状态")
    else:
        container.start()
        print(f"已启动容器 {container.name}")


# 停止容器
def stop_container(id):
    container = client.containers.get(id)
    if container.status == "exited":
        print("容器已处于停止状态")
    else:
        container.stop()
        print(f"已停止容器 {container.name}")


# 重启容器
def restart_container(id):
    container = client.containers.get(id)
    container.restart()
    print(f'已重启容器 {container.name}')


# 删除容器
def remove_container(id):
    container = client.containers.get(id)
    if container.status == "running":
        print("容器未停止，无法删除")
    else:
        container.remove()
        print(f'已删除容器 {container.name}')


#查看容器状态
def status_container(id):
    print("CONTAINER ID\t\tIMAGE\t\t\tSTATUS\t\tNAMES")
    container = client.containers.get(id)
    print(f"{container.short_id}\t{container.image}\t{container.status}\t\t{container.name}")

#设置健康检查
def healthcheck_container(id):
    container = client.containers.get(id)
    health_cmd = input("请设置检查容器健康状况的命令: ")
    print("以下请输入数字")
    health_interval = input("请设置两次健康检查的间隔: ")
    health_retries = input("请设置在多少次连续失败后，将容器状态视为unhealthy: ")
    health_timeout = input("请设置健康检查命令运行超时时间: ")
    healthcheck = docker.types.Healthcheck(
        test=[health_cmd],
        interval = int(health_interval),
        retries = int(health_retries),
        timeout = int(health_timeout)
        )
    container.update(healthcheck=healthcheck)


#检查容器
def check_health(container):
    data = container.attrs
    if 'Health' in data['State']:
        return data['State']['Health']['Status']
    else:
        return None

