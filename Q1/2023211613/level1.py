#安装Docker SDK
$ pip install docker

#使用from_env()函数实例化Docker客户端
import docker

client = docker.from_env()

#再引入yaml，方便后续操作
import yaml

#部署web后端服务与数据库
services = {
    'db': {
        'image': 'mysql:latest',
        'restart': 'always',
        'environment': {
            'MYSQL_DATABASE': 'mydatabase',
            'MYSQL_USER': 'user',
            'MYSQL_PASSWORD': 'password',
        }
    },
    'backend': {
        'build': './backend',
        'ports': ['5000:5000'],
        'restart': 'always',
        'depends_on': ['db'],
        'environment': {
            'DB_HOST': 'db',
            'DB_USER': 'user',
            'DB_PASSWORD': 'password',
            'DB_NAME': 'mydatabase'
        }
    }
}

#创建docker-compose文件
compose_config = {
    'version': '3',
    'services':services
}

#将docker-compose部署的内容写入创建的docker-compose文件中
with open('docker-compose.yaml','w')as file
	file.write(yaml.dump(compose_config))

#启动服务（运行docker-compose文件）
client.api.create_service(definition=compose_config)