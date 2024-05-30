import docker
import os

#创建客户端实例
client =docker.DockerClient(base_url='tcp://192.168.201.136:2375')

flag = 1#控制变量

def images():
    iamges = client.images.list()
    for image in iamges:
        print(f"ID:{str(image.id)[7:13]} rag:{image.tags}")
#啦镜像
def pull_image():
    iamge_name = input("请输入您要拉取的镜像名：")
    image_tag = input("请输入版本号：")
    client.images.pull(iamge_name, image_tag)
#跑容器
def run_image():
    a = int(input("后台运行还是交互式运行（后台请按0，交互请按1）："))
    image_name = input("run的镜像名：")
    tag = input("版本号：")
    container_name = input("容器起名为：")

    if a == 0:
        container = client.containers.run(
            f'{image_name}:{tag}',  # 镜像名称和标签
            'bash',  # 在容器内执行的命令
            detach=True,  # 后台运行容器
            name=container_name,  # 容器名称
            tty=True,  # 分配一个伪终端
            remove=False  # 容器退出时自动删除
        )
#销毁容器
def remove_container():

    container_name = input("输入要删除容器名：")
    try:
        container = client.containers.get(container_name)
    except:
        print(f"没找到{container_name}")
    else:
        container.remove(force=True)

#停止正在运行的容器
def stop_container():
    x = input("是否停止所有运行的容器y\\n:")
    if x == "y":
        container_name = input("输入要停止的容器名：")
        try :
            container = client.containers.get(container_name)
        except:
            print(f"没找到{container_name}容器")
        else:
            container.stop()
    else:
        for container in client.containers.list:
            print(container)
            container.stop()
#start container
def start_container():
    x = input("重启容器按1，启动容器按2:\n")
    if x== 1:
        container_name = input("请输入重启的容器名或id：")
        container = client.containers.get(container_name)
        container.restart()
    if x== 2:
        container_name = input("请输入要启动的容器：")
        container = client.containers.get(container_name)
        container.start()
#菜单界面
def menu():
    global flag
    print("————————菜单————————\n"
          "查看镜像请按1：\n"
          "拉取镜像请按2：\n"
          "run镜像请按3：\n"
          "remove container请按4：\n"
          "stop container请按5：\n"
          "清屏请按666:\n"
          "结束管理请按0：\n"
          
          "————————————————————")
    flag = int(input("请输入您的选择:"))
    if flag ==1:
        images()
    if flag ==2:
        pull_image()
    if flag == 3:
        run_image()
    if flag == 4:
        remove_container()
    if flag == 5:
        stop_container()
    if flag == 666:
        os.system('cls')
while flag:
    menu()
print("程序运行结束！！！")