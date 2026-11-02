#!/usr/bin/env bash
set -u

# 一键清理 TUN 接口与路由
# 用法:
#   sudo ./clear_tun.sh
# 强制杀进程:
#   sudo FORCE=1 ./clear_tun.sh

if [ "$(id -u)" -ne 0 ]; then
  echo "请以 root 或 sudo 权限运行此脚本"
  exit 1
fi

echo "=== 检测 TUN 接口 ==="
mapfile -t TUN_IFS < <(ip -o link show | awk -F': ' '{print $2}' | grep -E '^(tun|utun|clash)' 2>/dev/null || true)

if [ ${#TUN_IFS[@]} -eq 0 ]; then
  echo "未检测到常见 TUN 接口（tun*/utun*/clash*）"
else
  for ifname in "${TUN_IFS[@]}"; do
    echo
    echo ">>> 处理接口: $ifname"

    echo "- 当前接口状态"
    ip -d link show "$ifname" 2>/dev/null || echo "  无法读取接口详情"

    echo "- 列出接口上的路由"
    ip route show dev "$ifname" || echo "  无路由或无法读取"

    echo "- 清除接口路由"
    ip route flush dev "$ifname" 2>/dev/null && echo "  已 flush 路由" || echo "  flush 路由失败或无路由"

    echo "- 将接口置为 down"
    ip link set dev "$ifname" down 2>/dev/null && echo "  已 down" || echo "  down 失败或接口不存在"

    echo "- 尝试删除 TUN 设备"
    ip tuntap del dev "$ifname" mode tun 2>/dev/null && echo "  已通过 ip tuntap 删除" || \
      { ip link delete "$ifname" type tun 2>/dev/null && echo "  已通过 ip link delete 删除" || echo "  删除设备失败或设备由内核/程序管理"; }

    echo ">>> $ifname 处理完成"
  done
fi

echo
echo "=== 检查 /dev/net/tun 占用进程 ==="
if command -v lsof >/dev/null 2>&1; then
  PIDS=$(lsof -t /dev/net/tun 2>/dev/null || true)
else
  PIDS=""
fi

# 额外尝试按常见程序名查找
EXTRA_PIDS=$(pgrep -f 'clash|v2ray|openvpn|wireguard|wg-quick|tun' 2>/dev/null || true)

ALL_PIDS="$(printf "%s\n%s\n" "$PIDS" "$EXTRA_PIDS" | tr ' ' '\n' | awk 'NF' | sort -u)"

if [ -z "$ALL_PIDS" ]; then
  echo "未检测到占用 /dev/net/tun 的进程"
else
  echo "检测到以下进程可能占用 TUN 设备"
  printf "%s\n" $ALL_PIDS | xargs -r ps -o pid,uid,user,cmd -p
  if [ "${FORCE:-0}" = "1" ]; then
    echo
    echo "FORCE=1 已设置，尝试优雅终止这些进程（SIGTERM），若失败则强制杀死（SIGKILL）"
    for pid in $ALL_PIDS; do
      echo "- 发送 SIGTERM 到 PID $pid"
      kill "$pid" 2>/dev/null || true
    done
    sleep 2
    for pid in $ALL_PIDS; do
      if kill -0 "$pid" 2>/dev/null; then
        echo "- PID $pid 仍在，发送 SIGKILL"
        kill -9 "$pid" 2>/dev/null || true
      fi
    done
    echo "进程终止尝试完成"
  else
    echo
    echo "若要同时终止这些进程，请以 root 运行脚本并设置环境变量 FORCE=1"
    echo "示例: sudo FORCE=1 ./clear_tun.sh"
  fi
fi

echo
echo "=== 额外建议 ==="
echo "- 若使用 systemd 管理的服务（openvpn@, wg-quick@, clash 等），建议用 systemctl stop 服务名 来优雅停止"
echo "- 若某些 TUN 接口由 NetworkManager 或特定程序自动重建，需在对应程序/配置中禁用 TUN 或停止该程序"
echo
echo "清理完成"

