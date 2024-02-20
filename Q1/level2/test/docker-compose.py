from netkiller.docker import *
import os

nginx = Services('nginx')
nginx.image('nginx:latest')
nginx.container_name('nginx')
nginx.restart('always')
nginx.ports('80:80')
nginx.ports('5000:5000')
nginx.volumes(['./nginx.conf:/etc/nginx/nginx.conf'])

http1 =  Services('http-1')
http1.image('httpd')
http1.container_name('http-1')
http1.restart('always')
http1.volumes(['./test1:/usr/local/apache2/htdocs/'])


http2 =  Services('http-2')
http2.image('httpd')
http2.container_name('http-2')
http2.restart('always')
http2.volumes(['./test2:/usr/local/apache2/htdocs/'])


dockerfile = Dockerfile()
dockerfile.image('python:3.8')
dockerfile.workdir('/app')
dockerfile.run('pip install flask mariadb')
dockerfile.cmd(["python", "app.py"])
dockerfile.save('./Dockerfile')

python1 = Services('python-1')
python1.build(dockerfile)
python1.container_name('python-1')
python1.restart('always')
python1.volumes(['./test2:/app'])

python2 = Services('python-2')
python2.build(dockerfile)
python2.container_name('python-2')
python2.restart('always')
python2.volumes(['./test1:/app'])

mariadb = Services('mariadb')
mariadb.image('mariadb:latest')
mariadb.container_name('mariadb')
mariadb.restart('always')
mariadb.volumes(['./data:/var/lib/mysql', './backup.sql:/docker-entrypoint-initdb.d/backup.sql'])
mariadb.environment('MARIADB_ROOT_PASSWORD=1234')

compose = Composes('development')
compose.version('3.9')
compose.services(nginx)
compose.services(python1)
compose.services(python2)
compose.services(http1)
compose.services(http2)
compose.services(mariadb)


compose.save('./docker-compose.yaml')
os.system('docker-compose -f docker-compose.yaml up -d')
