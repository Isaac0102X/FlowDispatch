#!/bin/bash
echo "=== 检查 TUN 网卡设备 ==="
ip link show | grep -E 'tun|utun|clash' || echo "未检测到 TUN 虚拟网卡"

echo -e "\n=== 检查路由表是否指向 TUN ==="
ip route | grep -E 'tun|utun|clash' || echo "未检测到路由表指向 TUN"

#echo -e "\n=== 检查相关进程 (clash/v2ray/openvpn) ==="
#ps aux | grep -E 'clash|v2ray|openvpn|tun' | grep -v grep || echo "未检测到相关进程"

echo -e "\n=== 检查内核日志 ==="
dmesg | grep tun || echo "未检测到 TUN 内核日志"

