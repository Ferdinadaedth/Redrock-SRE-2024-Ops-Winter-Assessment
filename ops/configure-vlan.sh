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