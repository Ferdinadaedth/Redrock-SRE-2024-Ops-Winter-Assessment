# level0



level0.py，通过docker的远程api访问，达到远程连接，

## 功能

实现了远程拉取镜像，run镜像，查看本地镜像仓库，开始或重启容器，以及删除容器与镜像的基本炒作。

## 缺点

还是需要手动输入控制，达不到自动化。

## 存在的bug

该程序没有对用户输入作检查，以及没有详细捕获相应的错误。所以如果用户输入了不合法的语法。则可能导致程序崩溃瘫痪。



# level 1

我使用了Python轻量级的webflask框架写的简单的登录网页。

使用了mysql-connector包与数据库交互。可以把用户输入的信息存入数据库中。

使用python脚本，可以类似于docker-compose.yaml文件脚本执行

```
#先看看docker-compose.yml
version: '3.9'
services:
  flask-app:
    build: .
    ports:
      - "5000:5000"
    restart: always
    environment:
      - FLASK_APP=app.py
      - FLASK_ENV=development
      - DATABASE_URL=mysql+pymysql://root:root@db:3306/level1
    volumes:
      - .:/app
    depends_on:
      - db
    command: flask run --host=0.0.0.0
    
    
  db:
    image: mysql:5.7
    restart: always
    environment:
      - MYSQL_ROOT_PASSWORD=root
      - MYSQL_DATABASE=level1
      - MYSQL_USER=user
      - MYSQL_PASSWORD=root
    ports:
      - "3306:3306"
    volumes:
      - /mysql-data:/var/lib/mysql
    command: --default-authentication-plugin=mysql_native_password --init-file=/docker-entrypoint-initdb.d/init.sql
```

转化为Python：

```
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
app_config = {
    'image': 'flask_app',
    'ports': ['5000:5000'],
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
},

# 创建 Flask 应用程序和 MySQL 数据库容器
# **传递字典
flask_app = client.containers.create(**app_config)
db = client.containers.create(**db_config)

# 启动容器
db.start()

time.sleep(10)

flask_app.start()
```

运行后，需要注意开放5000端口



# level2

通过修改level1的docker-compose文件,分别跑了使app1,app2,app3三个容器,来模拟三个服务。并要达到负载均衡。

思路：

用docker跑一个nginx，并挂载数据卷。

修改对应的配置文件，达到目的。

```

#配置三个app
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



flask_app1 = client.containers.create(**app1)
flask_app2 = client.containers.create(**app2)
flask_app3 = client.containers.create(**app3)
db = client.containers.create(**db_config)

#配置nginx
nginx1 = {
    'name': 'nginx',
    'image': 'nginx:latest',
    'ports': ['80:80'],
    'volumes': ['/app/conf.d', '/etc/nginx/conf.d'],
    'command': 'nginx -g "daemon off;"'  # 确保 Nginx 守护进程运行
}

#配置文件defualt.conf

upstream backend{
    server app1:9001 weight=1;
    server app2:9002 weight=1;
    server app3:9003 weight=1;
}
server {
    listen 80;
    server_name 192.168.201.140;
    location /{
        proxy_pass http://backend;
    }
}

```



