## Level1

题目中明确说明在OpenWrt系统的软路由上操作，所以我用了OpenWrt自带的UCI命令行工具来配置网络。写了一个简单的Bash脚本。(configure-vlan.sh)

```
#!/bin/bash

# VLAN ID数组，按照题目中给出的网段确定
VLAN_IDS=(10 202 219 222)

# 网络接口名称，假设为eth0
INTERFACE="eth0"

# 遍历VLAN_IDS数组
for VLAN_ID in "${VLAN_IDS[@]}"
do
    # 使用UCI工具创建一个用于VLAN的网络接口
    uci set network.vlan${VLAN_ID}="interface"
    # 指定接口名称
    uci set network.vlan${VLAN_ID}.ifname="${INTERFACE}.${VLAN_ID}"
    # 设置接口使用DHCP协议获取IP地址
    uci set network.vlan${VLAN_ID}.proto="dhcp"
    # 指定接口类型为桥接
    uci set network.vlan${VLAN_ID}.type="bridge"
done

# 提交上面的UCI配置更改
uci commit network

# 重启网络服务以应用更改后的配置文件
/etc/init.d/network reload

# 路由设置函数
setup_routes() {
    # 为每个VLAN接口添加指定的路由，使得接入这些VLAN的设备能够访问其他特定的网络网段

    # 添加路由到10.18.0.0/12网段，通过VLAN 10接口，下同
    ip route add 10.18.0.0/12 dev ${INTERFACE}.10
    ip route add 202.202.32.0/20 dev ${INTERFACE}.202
    ip route add 219.153.62.64/26 dev ${INTERFACE}.219
    ip route add 222.177.140.0/25 dev ${INTERFACE}.222
}

# 路由持久化函数
persist_routes() {
    # 将路由设置命令写入rc.local中，实现系统启动时自动配置路由规则
    echo "setup_routes" >> /etc/rc.local
}

# 设置路由
setup_routes
# 持久化路由配置
persist_routes

# 输出完成提示
echo "VLAN配置成功!"

```

### Getting-Started

给予脚本执行权限后以Root权限运行即可。

```
chmod +x configure-vlan.sh
sudo sh configure-vlan.sh
```



## Level2

同样用Bash脚本解决。(configure-vm-network.sh)

```
#!/bin/bash

# 环境变量
EXTERNAL_INTERFACE="eth0"     # 连接到外部网络的网络接口名称，假设为eth0
VM_BRIDGE="vmbr1"             # 虚拟机网桥名称
VM_NETWORK="192.168.5.0/24"   # 虚拟机网络的网段
VM_GATEWAY="192.168.5.1"      # 网关地址
VM_NETMASK="255.255.255.0"    # 子网掩码

# 网桥接口创建函数
setup_bridge() {
    ip link add name $VM_BRIDGE type bridge             # 创建一个新的网桥接口
    ip addr add $VM_GATEWAY/$VM_NETMASK dev $VM_BRIDGE  # 为网桥接口分配IP地址和子网掩码
    ip link set $VM_BRIDGE up                           # 激活网桥接口
}

# NAT和防火墙规则配置函数
setup_firewall() {
    # 在POSTROUTING链上添加MASQUERADE规则，使得从VM_BRIDGE出来的数据包伪装成来自EXTERNAL_INTERFACE的数据包
    iptables -t nat -A POSTROUTING -o $EXTERNAL_INTERFACE -j MASQUERADE
    # 允许从VM_BRIDGE到EXTERNAL_INTERFACE的数据包转发
    iptables -A FORWARD -i $VM_BRIDGE -o $EXTERNAL_INTERFACE -j ACCEPT
    # 允许已建立连接的数据包在EXTERNAL_INTERFACE和VM_BRIDGE之间转发
    iptables -A FORWARD -o $VM_BRIDGE -i $EXTERNAL_INTERFACE -m state --state RELATED,ESTABLISHED -j ACCEPT
}

# 防火墙和网桥设置持久化函数
persist_settings() {
    # 将NAT和防火墙配置写入rc.local
    echo "setup_firewall" >> /etc/rc.local
    # 将网桥配置写入rc.local
    echo "ip link add name $VM_BRIDGE type bridge" >> /etc/rc.local
    echo "ip addr add $VM_GATEWAY/$VM_NETMASK dev $VM_BRIDGE" >> /etc/rc.local
    echo "ip link set $VM_BRIDGE up" >> /etc/rc.local
}

# 创建网桥接口
setup_bridge

# 配置NAT和防火墙规则
setup_firewall

# 使配置持久化
persist_settings

# 输出完成提示
echo "网络配置成功!"
```

### Getting-Started

给予脚本执行权限后以Root权限运行即可。

```
chmod +x configure-vm-network.sh
sudo sh configure-vm-network.sh
```

------

考虑到涉及iptables的操作确实比较抽象，我再详细解释一下上面的persist_settings()函数。

```
iptables -t nat -A POSTROUTING -o $EXTERNAL_INTERFACE -j MASQUERADE
```

这条命令启用了IP伪装，使得从虚拟机网络发出的所有数据包，在通过外部网络接口（就是上面假设的eth0）发送到互联网上时，都会伪装成外部网络接口的IP地址。（就是把来自虚拟机网络的内网IP出站数据包替换成公网IP地址）

```
iptables -A FORWARD -i $VM_BRIDGE -o $EXTERNAL_INTERFACE -j ACCEPT
```

这条命令告诉iptables允许所有"从虚拟机网桥接口（`$VM_BRIDGE`）进来，并且将要通过外部接口(eth0)发送出去"的数据包。`FORWARD` 链用于处理被路由的数据包。 `-i` 参数指定了数据包的入口接口， `-o` 参数指定了数据包的出口接口。`-j ACCEPT` 表示允许这些数据包通过。

```
iptables -A FORWARD -o $VM_BRIDGE -i $EXTERNAL_INTERFACE -m state --state RELATED,ESTABLISHED -j ACCEPT
```

最后一条命令指定iptables只允许已建立（ESTABLISHED）和相关联（RELATED）的连接的数据包返回到虚拟机网桥接口($VM_BRIDGE)，从而实现题目中要求的网络环境隔离。
