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