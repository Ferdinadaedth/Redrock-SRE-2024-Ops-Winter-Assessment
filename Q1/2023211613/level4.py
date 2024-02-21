import time
from docker_compose import compose

#指向docker-compose.yml文件
compose = compose(project_name='ioo',file='docker-compose.yml')

#启动docker-compose服务
compose.up()

#定义健康检查函数
def is_container_healthy(container_name):  
    container = compose.containers.get(container_name)  
    if container.is_running:  
        return container.is_healthy  
    return False  

# 监控健康状态并重启不健康的容器  
while True:  
    for container_name in compose.containers.list_names():  
         if not is_container_healthy(container_name):  
            print(f"容器 {container_name} 不健康, 正在重新启动")  
            # 停止并重启容器  
            compose.containers.get(container_name).restart()
             time.sleep(5) 
        else:  
            print(f"容器{container_name}健康")
            
    time.sleep(20)
    