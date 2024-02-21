import docker

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
    print("CONTAINER ID\t\tIMAGE\t\t\tSTATUS\t\tNAMES")

    for container in containers:
        print(f"{container.short_id}\t{container.image}\t{container.status}\t\t{container.name}")

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

while 1:
    a=input("""
1.创建一个新的容器
2.对现有容器进行操作
q.退出
请选择:\n""")
    if a=="1":
        list_images()
        image=input("请选择镜像: ")
        if test_image(image):
            continue
        name=input("请输入容器名称: \n")
        create_container(image, name)
    elif a=="2":
        while 1:
            list_container()
            id=input("请选择容器,或输入q返回上一级\n")
            if id=="q":
                break
            result=test_container(id)
            if result ==0:
                print("请重新输入\n")
            while result:
                b=input("""
1.启动容器
2.停止容器
3.重启容器
4.删除容器
5.查看容器状态
q.返回上一级
请选择:\n""")
                if b=="1":
                    start_container(id)
                elif b=="2":
                    stop_container(id)
                elif b=="3":
                    restart_container(id)
                elif b=="4":
                    remove_container(id)
                    break
                elif b=="5":
                    status_container(id)
                elif b=="q":
                    break
                else:
                    print("请重新输入\n")
    elif a=="q":
        break
    else:
        print("请重新输入\n")

