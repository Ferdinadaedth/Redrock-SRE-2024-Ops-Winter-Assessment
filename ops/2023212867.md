# 第一题

*lingkep刚刚学习网络，光看理论太枯燥了，一层一层的概念让他无从下手，且不知道在实际的应用，于是他决定练练手。他已经搭建了一个局域网自己使用，可以正常上网，网段为172.18.100.1/24，网关机ip为172.18.100.1。
①他配置好的设备可以接入校园网，他想直接在他的软路由openwrt上接入，只需配置vlan ID就可以dhcp到校园网ip，为了访问其他网段，需要写好路由，有web界面但他不想用。请你帮他写好以上配置，接口名你自己定义。（网段分别为10.18.0.0/12，202.202.32.0/20，219.153.62.64/26，222.177.140.0/25）*

## 基础概念

**局域网**

局域网（Local Area Network, LAN），又称内网。 **指覆盖局部区域（如办公室或楼层）的计算机网络**。

**网段**

网段（network segment）**一般指一个计算机网络中使用同一物理层设备（传输介质，中继器，集线器等）能够直接通讯的那一部分**。

**网关机IP(或默认网关)**

默认网关（Default Gateway），也叫缺省网关，**是子网与外网连接的设备，通常是一个路由器**。当一台计算机发送信息时，根据发送信息的目标地址，通过子网掩码来判定目标主机是否在本地子网中，如果目标主机在本地子网中，则直接发送即可。如果目标不在本地子网中则将该信息送到默认网关/路由器，由路由器将其转发到其他网络中，进一步寻找目标主机。

## 操作

1. 连接上软路由.

2. ```shell
   vi /etc/config/network
   ```

3. 假设校园网的网关IP为`172.18.100.1`.

4. ```shell
   config interface 'vlan100'
       option type 'bridge' #桥接模式
       option ifname 'eth0.100'
       option proto 'dhcp'
   ```

5. ```shell
   config route
       option interface 'vlan100'
       option target '10.18.0.0'
       option netmask '255.240.0.0'
       option gateway '172.18.100.1'
       
   config route
       option interface 'vlan100'
       option target '202.202.32.0'
       option netmask '255.255.240.0'
       option gateway '172.18.100.1'
   
   config route
       option interface 'vlan100'
       option target '219.153.62.64'
       option netmask '255.255.255.192'
       option gateway '172.18.100.1'
   
   config route
       option interface 'vlan100'
       option target '222.177.140.0'
       option netmask '255.255.255.128'
       option gateway '172.18.100.1'
   ```

6. `:wq`

7. ```shell
   /etc/init.d/network reboot now
   ```

# 第二题

*②他想在服务器上创建十台Debian虚拟机(192.168.5.2-11)给朋友用，要求网关已有网关机是同一个，并且这十台虚拟机要能上网，同时网络环境要和原来网络环境隔离(新网段流量无法进入旧网段)，分别需在网关机上做什么配置？同时，他想把某些配置命令持久化，请你帮他编写网络配置脚本与开机自启脚本。*

1. 首先进行隔离.

设立`114.214.0.0/24`网段作为新的子网,新的网关机的地址为`114.214.0.1`,子网掩码为`255.255.255.0.`	

> 为了让新网段流量无法进入旧网段,选择一个完全不同的私有IP地址范围作为新的子网.

2. 配置网络脚本

```shell
#!/bin/bash

ip addr add 114.214.0.0/24 dev eth0
sysctl -w net.ipv4.ip_forward=1
echo "net.ipv4.ip_forward=1" >> /etc/sysctl.conf #配置保存
```

3. 大便的配置和脚本配置

 网关IP设为`114.214.0.1`.

```shell
sudo vim /etc/network/interfaces
auto eth0
iface eth0 inet static
    address 114.214.0.2~11
    netmask 255.255.255.0
    gateway 114.214.0.1
    dns-nameservers 8.8.8.8
```

4. 开机自启

```shell
vim network01.service
[Unit]
Description=Network Configuration

[Service]
ExecStart=/usr/bin/network01.sh

[Install]
WantedBy=default.target
```

5. 配置

接下来依次执行以下命令:

```shell
vim network02.sh

#!/bin/bash

ip addr add 114.214.0.0/24 dev eth0
sysctl -w net.ipv4.ip_forward=1
echo "net.ipv4.ip_forward=1" >> /etc/sysctl.conf 
```

````shell
sudo cp network01.service /etc/systemd/system/
sudo cp network02.sh /usr/bin/
sudo chmod +x /usr/bin/network02.sh
sudo systemctl enable network01.service
````

