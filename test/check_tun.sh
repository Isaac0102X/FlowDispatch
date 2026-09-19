#!/bin/bash

echo "=== 1. 检查活跃的 TUN 虚拟网卡设备 ==="
# 排除系统自带的 tunl0 接口，寻找状态为 UP 或常见代理命名的 TUN 网卡
TUN_DEV=$(ip link show | grep -E 'tun[0-9]+|utun|clash|sing|meta' | grep -v 'tunl0')

if [ -n "$TUN_DEV" ]; then
    echo "$TUN_DEV"
else
    echo "未检测到 TUN 虚拟网卡"
fi

echo -e "\n=== 2. 测试发往公网流量的实际路由指向 ==="
# 使用 ip route get 模拟查询 1.1.1.1 的出口网卡（可穿透策略路由）
ROUTE_INFO=$(ip route get 1.1.1.1 2>/dev/null)
echo "$ROUTE_INFO"

if echo "$ROUTE_INFO" | grep -qE 'dev (tun|utun|clash|sing|meta)'; then
    echo -e "\033[32m[结果] TUN 模式接管流量成功！\033[0m"
else
    echo -e "\033[31m[结果] 当前默认流量未经过 TUN 设备。\033[0m"
fi

echo -e "\n=== 3. 检查自定义策略路由规则 (ip rule) ==="
# 检查是否存在非标准的路由规则 (Clash/Sing-box 常用于重定向流量)
CUSTOM_RULES=$(ip rule show | grep -vE 'lookup (main|default|local)')

if [ -n "$CUSTOM_RULES" ]; then
    echo "$CUSTOM_RULES"
else
    echo "未检测到残留的策略路由规则"
fi
