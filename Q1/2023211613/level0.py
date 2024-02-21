#安装Docker SDK
$ pip install docker

#使用from_env()函数实例化Docker客户端
import docker

client = docker.from_env()

#创建容器
def create_container(image,name,**kwargs)
	return client.containers.run(image,name=name,**kwargs)

#启动容器
def start_container(container):
  return container.start()

#停止容器
def stop_container(container):
  return container.stop() 

#重启容器
def restart_container(container):
  return container.restart()

#删除（销毁）容器
def remove_container(container):
  return container.remove()