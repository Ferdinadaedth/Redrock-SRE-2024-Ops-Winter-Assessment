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
    echo "iptables -t nat -A POSTROUTING -o $EXTERNAL_INTERFACE -j MASQUERADE" >> /etc/rc.local
    echo "iptables -A FORWARD -i $VM_BRIDGE -o $EXTERNAL_INTERFACE -j ACCEPT" >> /etc/rc.local
    echo "iptables -A FORWARD -o $VM_BRIDGE -i $EXTERNAL_INTERFACE -m state --state RELATED,ESTABLISHED -j ACCEPT" >> /etc/rc.local
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