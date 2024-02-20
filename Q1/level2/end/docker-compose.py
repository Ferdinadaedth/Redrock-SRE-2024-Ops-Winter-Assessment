from netkiller.docker import *
import os
import re

num1 = int(input("请输入前端服务器数量"))
num2 = int(input("请输入后端服务器数量"))

with open('nginx.conf', 'r+') as f:
    lines = f.readlines()
    f.seek(0)
    f.truncate()

    for line in lines:
        if not re.match(r'\s*server http-\d+:80;\s*', line) and not re.match(r'\s*server python-\d+:5000;\s*', line):
            f.write(line)
        if line.strip() == 'upstream myserver-2 {':
            for i in range(num2):
                f.write('    server python-{}:5000;\n'.format(i+1))
        if line.strip() == 'upstream myserver-1 {':
            for i in range(num1):
                f.write('    server http-{}:80;\n'.format(i+1))

compose = Composes('development')
compose.version('3.9')

nginx = Services('nginx')
nginx.image('nginx:latest')
nginx.container_name('nginx')
nginx.restart('always')
nginx.ports('80:80')
nginx.ports('5000:5000')
nginx.volumes(['./nginx.conf:/etc/nginx/nginx.conf'])
compose.services(nginx)

dockerfile = Dockerfile()
dockerfile.image('python:3.8')
dockerfile.workdir('/app')
dockerfile.run('pip install flask mariadb')
dockerfile.cmd(["python", "app.py"])
dockerfile.save('./Dockerfile')

mariadb = Services('mariadb')
mariadb.image('mariadb:latest')
mariadb.container_name('mariadb')
mariadb.restart('always')
mariadb.volumes(['./data:/var/lib/mysql', './backup.sql:/docker-entrypoint-initdb.d/backup.sql'])
mariadb.environment('MARIADB_ROOT_PASSWORD=1234')
compose.services(mariadb)

for i in range(num1) :

    http =  Services('http-'+str(i+1))
    http.image('httpd')
    http.container_name('http-'+str(i+1))
    http.restart('always')
    http.volumes(['.:/usr/local/apache2/htdocs/'])
    compose.services(http)

for i in range(num2) :

    python = Services('python-'+str(i+1))
    python.build(dockerfile)
    python.container_name('python-'+str(i+1))
    python.restart('always')
    python.volumes(['.:/app'])
    compose.services(python)

compose.save('./docker-compose.yaml')
os.system('docker-compose -f docker-compose.yaml up -d')
