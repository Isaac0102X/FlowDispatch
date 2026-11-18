
参见
https://deepwiki.com/search/stl_8d7e8807-a74e-4998-96b0-beb8872f7023

## 附录：三控制器协作关系  
  
```plantuml  
@startuml Controllers  
hide empty description  
  
[CoreController] as core  
[TunController] as tun  
[SystemProxyController] as sys  
  
core -down-> tun : "internal tun 从属于\ncore配置层状态"  
tun -up-> core : "core崩溃/重建 →\nTUN转入Stopping"  
sys ..> core : "仅引用 inbound_socks_port\n(无RPC，无生命周期耦合)"  
  
note bottom of sys  
  系统代理是唯一与core状态机  
  正交的控制器：  
  spmode_system_proxy 不进 core 配置  
end note  
  
note bottom of tun  
  退出序列由状态机驱动：  
  sys→Released && tun→Down && core→Stopped  
  全部就绪才允许 quit，  
  替代 mu_exit.tryLock 重试 hack  
end note  
@enduml

``` 

### 落地要点  
  
- UI 层只发 `enableRequest()`/`stopRequest()` 意图事件，控制器用信号广播状态，`refresh_status` 为纯渲染；  
- `MW_dialog_message("CoreStarted,"+id)` 等字符串协议全部替换为类型化状态事件，CoreProcess 不再直接命令 MainWindow；  
- 每个 `Starting`/`Stopping` 态的超时作为状态内 timer 建模，消掉散落的 `setTimeout`/`MessageBoxTimer`；  
- 三个控制器启动时各自 reconcile 一遍（core 端口占用、TUN/iptables 残留、系统代理指纹），把"崩溃残留"变成一等工作流。


