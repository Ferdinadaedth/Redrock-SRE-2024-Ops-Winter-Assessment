from netkiller.docker import *
import os

nginx = Services('nginx')
nginx.image('nginx:latest')
nginx.container_name('nginx')
nginx.restart('always')
nginx.ports('80:80')
nginx.volumes(['.:/usr/share/nginx/html'])

dockerfile = Dockerfile()
dockerfile.image('python:3.8')
dockerfile.workdir('/app')
dockerfile.run('pip install flask mariadb')
dockerfile.cmd(["python", "app.py"])
dockerfile.save('./Dockerfile')

python = Services('python')
python.build(dockerfile)
python.container_name('python')
python.restart('always')
python.volumes(['.:/app'])
python.ports('5000:5000')

mariadb = Services('mariadb')
mariadb.image('mariadb:latest')
mariadb.container_name('mariadb')
mariadb.restart('always')
mariadb.volumes(['./data:/var/lib/mysql', './backup.sql:/docker-entrypoint-initdb.d/backup.sql'])
mariadb.environment('MARIADB_ROOT_PASSWORD=1234')

compose = Composes('development')
compose.version('3.9')
compose.services(nginx)
compose.services(python)
compose.services(mariadb)


compose.save('./docker-compose.yaml')
os.system('docker-compose -f docker-compose.yaml up -d')
