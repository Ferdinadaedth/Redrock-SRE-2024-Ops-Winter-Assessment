import docker
import time
from docker.models.containers import Container

from docker.models.images import Image

client = docker.from_env()  # 连接到docker客户端

# 定义dockerfile路径与image的名称
path = './dockerfile'
image_name = 'flask_app'

# 提前构建镜像
client.images.build(path='/app/flaskproject/app.py', dockerfile=path, tag=image_name)  # 绝对路径

# 将配置定义成字典

# 跑三个容器，分别映射到主机三个不同的端口。
#三个容器对应模拟三个服务器
app1 = {
    'container_name': 'app1',
    'image': 'flask_app',
    'ports': ['9001:5000'],
    'environment': {
        'FLASK_APP': 'app.py',
        'FLASK_ENV': 'development',
        'DATABASE_URL': 'mysql+pymysql://root:root@db:3306/level1'
    },
    'volumes': ['/app:/app'],
    'depends_on': ['db']
}
app2 = {
    'container_name': 'app2',
    'image': 'flask_app',
    'ports': ['9002:5000'],
    'environment': {
        'FLASK_APP': 'app.py',
        'FLASK_ENV': 'development',
        'DATABASE_URL': 'mysql+pymysql://root:root@db:3306/level1'
    },
    'volumes': ['/app:/app'],
    'depends_on': ['db']
}
app3 = {
    'container_name': 'app3',
    'image': 'flask_app',
    'ports': ['9003:5000'],
    'environment': {
        'FLASK_APP': 'app.py',
        'FLASK_ENV': 'development',
        'DATABASE_URL': 'mysql+pymysql://root:root@db:3306/level1'
    },
    'volumes': ['/app:/app'],
    'depends_on': ['db']
}

db_config = {
    'image': 'mysql:5.7',
    'ports': ['3306:3306'],
    'environment': {
        'MYSQL_DATABASE': 'level1',
        'MYSQL_PORT': '3306',
        'MYSQL_USER': 'root',
        'MYSQL_PASSWORD': 'root',
    },
    'volumes': ['/mysql-data:/var/lib/mysql'],
    'command': '--default-authentication-plugin=mysql_native_password --init-file=/docker-entrypoint-initdb.d/init_db.sql'
}

# 创建 Flask 应用程序和 MySQL 数据库容器
# **传递字典
flask_app1 = client.containers.create(**app1)
flask_app2 = client.containers.create(**app2)
flask_app3 = client.containers.create(**app3)
db = client.containers.create(**db_config)

# 启动容器
db.start()

time.sleep(10)

flask_app1.start()
flask_app2.start()
flask_app3.start()

# 配置nginx反向代理这三个服务器

nginx1 = {
    'name': 'nginx',
    'image': 'nginx:latest',
    'ports': ['80:80'],
    'volumes': ['/app/conf.d', '/etc/nginx/conf.d'],
    'command': 'nginx -g "daemon off;"'  # 确保 Nginx 守护进程运行
}
flask_nginx = client.containers.create(**nginx1)
flask_nginx.start()
