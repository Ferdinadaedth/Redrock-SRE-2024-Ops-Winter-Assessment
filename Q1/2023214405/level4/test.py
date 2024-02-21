from support import *
import pprint
import time

client = docker.from_env()

container = client.containers.run("nginx", name="nginx", detach=True)
time.sleep(5)
data = container.attrs
pprint.pprint(data)
