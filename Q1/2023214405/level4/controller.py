from support import *
import docker
import time
import os

os.system('python healthcheck.py &')
client = docker.from_env()


while 1:
    a=input("""
1.创建一个新的容器
2.对现有容器进行操作
q.退出
请选择:\n""")
    if a=="1":
        list_images()
        image=input("请选择镜像, 或输入s拉取新镜像: ")
        if image=='s':
            print(f"请登入账户")
            os.system('docker login')
            image=input("拉取新镜像: ")
            print(f"正在拉取镜像{image}")
            try:
                client.images.pull(f'{image}')
                print(f"成功拉取 {image} 镜像。")
            except Exception as e:
                print(f"拉取镜像时出现错误: {e}")
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


cmd = 'ps aux | grep healthcheck.py'
output = os.popen(cmd).read()
pid = int(output.split()[1])
print(f'{pid}')
os.system(f'kill {pid}')

