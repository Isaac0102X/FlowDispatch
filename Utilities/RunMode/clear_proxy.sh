#!/bin/bash
echo "=== 清理环境变量代理 ==="
unset http_proxy https_proxy HTTP_PROXY HTTPS_PROXY no_proxy NO_PROXY ALL_PROXY
echo "已 unset 环境变量代理"

# 永久删除环境变量配置
for file in ~/.bashrc ~/.profile /etc/environment; do
    if [ -f "$file" ]; then
        sudo sed -i '/proxy/d;/PROXY/d' "$file"
        echo "已清理 $file 中的代理配置"
    fi
done

echo -e "\n=== 清理 GNOME 代理设置 ==="
if command -v gsettings >/dev/null; then
    gsettings set org.gnome.system.proxy mode 'none'
    gsettings set org.gnome.system.proxy.http host ''
    gsettings set org.gnome.system.proxy.http port 0
    gsettings set org.gnome.system.proxy.https host ''
    gsettings set org.gnome.system.proxy.https port 0
    gsettings set org.gnome.system.proxy.ftp host ''
    gsettings set org.gnome.system.proxy.ftp port 0
    gsettings set org.gnome.system.proxy.socks host ''
    gsettings set org.gnome.system.proxy.socks port 0
    echo "已清理 GNOME 所有代理键值"
else
    echo "未检测到 GNOME"
fi

echo -e "\n=== 清理 KDE/Plasma 代理设置 ==="
if [ -f ~/.config/kioslaverc ]; then
    sed -i '/Proxy/d' ~/.config/kioslaverc
    echo "已清理 KDE 配置文件中的代理"
else
    echo "未检测到 KDE 配置文件"
fi

#echo -e "\n=== 清理 APT 代理设置 ==="
#if [ -f /etc/apt/apt.conf ]; then
#    sudo sed -i '/Proxy/d' /etc/apt/apt.conf
#    echo "已清理 apt.conf 中的代理"
#else
#    echo "未检测到 apt.conf"
#fi

#echo -e "\n=== 清理 Git 全局代理设置 ==="
#git config --global --unset http.proxy
#git config --global --unset https.proxy
#echo "已清理 Git 全局代理"

echo -e "\n=== 重启 NetworkManager ==="
sudo systemctl restart NetworkManager
echo "已重启 NetworkManager，代理清理完成"

