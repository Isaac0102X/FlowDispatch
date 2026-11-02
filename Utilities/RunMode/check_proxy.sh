#!/bin/bash
echo "=== 环境变量代理 ==="
env | grep -i proxy

echo -e "\n=== GNOME 代理设置 ==="
if command -v gsettings >/dev/null; then
    gsettings list-recursively org.gnome.system.proxy | grep -E 'mode|host|port'
else
    echo "未检测到 gsettings (GNOME)"
fi

echo -e "\n=== KDE/Plasma 代理设置 ==="
if command -v kwriteconfig5 >/dev/null; then
    grep -E 'Proxy' ~/.config/kioslaverc 2>/dev/null || echo "未检测到 KDE 配置"
else
    echo "未检测到 kwriteconfig5 (KDE)"
fi

#echo -e "\n=== APT 代理设置 ==="
#grep -i proxy /etc/apt/apt.conf 2>/dev/null || echo "未检测到 apt.conf 中的代理"

echo -e "\n=== Git 全局代理设置 ==="
git config --global --get http.proxy
git config --global --get https.proxy

echo -e "\n=== NetworkManager 代理设置 ==="
nmcli connection show | grep -i proxy || echo "未检测到 NetworkManager 代理"

