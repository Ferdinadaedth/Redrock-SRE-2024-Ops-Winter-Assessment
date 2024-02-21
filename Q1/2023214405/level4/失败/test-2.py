from support import *
import docker
import time
import os
import subprocess

#p=subprocess.Popen(['python', 'healthcheck.py'],stdin=subprocess.PIPE, stdout=subprocess.PIPE)
#p=subprocess.Popen(['python', 'healthcheck.py'], stdout=subprocess.PIPE)
p=subprocess.Popen(['python', 'healthcheck.py'],stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL)



while 1:
    a=input("""
1.创建一个新的容器
2.对现有容器进行操作
q.退出
请选择:\n""")
p.kill()
