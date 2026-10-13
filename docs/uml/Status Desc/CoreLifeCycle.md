
设计目标：将当前散落在各处的零乱隐式状态代码，收敛为显式状态机。UI 层只发送意图事件（`enableRequest()`/`stopRequest()`），控制器持有状态并通过信号广播。
  
---  
  
## Core 进程生命周期（CoreController）  
  
### 设计动机  
  
现有代码中"core 进程状态"有两个独立维度被压在一起：**进程活没活** 和 **gRPC 配置跑没跑**。状态图将两层拆开，每个状态对应一个明确的运行时事实。  
  
### 状态图  

```plantuml  
@startuml CoreController  
hide empty description  
note "两阶段：QProcess拉起 ≠ gRPC可用\n就绪信号当前靠 stdout 字符串匹配\n\"grpc server listening\"" as N1  
  
[*] --> Stopped  
  
Stopped --> Spawning : spawnRequest()  
Spawning --> GrpcReady : "stdout: grpc server listening"  
Spawning --> SpawnFailed : "FailedToStart /\nlisten fail"  
  
SpawnFailed --> Stopped : "notify error / 不重试"  
note right of SpawnFailed  
  现在 failed_to_start 路径不重试，  
  而 "failed to serve" 走 kill→崩溃路径，  
  重试策略不一致，显式拆开  
end note

GrpcReady --> ConfigStarting : "startProfile(id)"  
ConfigStarting --> ConfigRunning : "RPC Start ok"  
ConfigStarting --> GrpcReady : "RPC Start fail"  
  
ConfigRunning --> ConfigStopping : "stopRequest(id) / 切换节点"  
ConfigStopping --> GrpcReady : "RPC Stop ok / 超时兜底"  
ConfigRunning --> Reconfiguring : "更换端口/路由等\n需重启配置的请求"  
Reconfiguring --> ConfigStarting  
  
GrpcReady --> Stopping : "stopCoreRequest / 退出"  
ConfigRunning --> Stopping : "退出（先RPC Stop）"  
ConfigStopping --> Stopping : "退出插队"  
Stopping --> Stopped : "进程退出 / kill 超时兜底"  
  
GrpcReady --> CrashBackoff : "进程意外退出"  
ConfigRunning --> CrashBackoff : "进程意外退出"  
CrashBackoff --> Spawning : "retry && 频率未超限"  
CrashBackoff --> Stopped : "10s内重复崩溃 → 放弃"  
note bottom of CrashBackoff  
  吸收现在的 coreRestartTimer +  
  setTimeout(1s) + restarting flag  
  窗口期内新请求入队而非丢弃  
end note  

state PendingProfile  
Spawning --> PendingProfile : "start_profile_when_core_is_up >= 0"  
PendingProfile --> ConfigStarting : "core ready 后自动补发"  
@enduml  
```  
  
### 状态说明  
  
| 状态 | 含义 | 对应现有代码 |  
|---|---|---|  
| `Stopped` | core 进程未运行。程序刚启动、core 退出后、崩溃放弃重试后均在此 | `core_process` 为 null / `core_running=false` 且进程 NotRunning |  
| `Spawning` | 已调用 `QProcess::start()`，进程拉起中，但尚未确认可用 | `ExternalProcess::Start()` 之后、读到 "grpc server listening" 之前 |  
| `GrpcReady` | 进程存活，gRPC 端口在监听，但没有配置在跑（空转） | `core_running=true` 且 `started_id < 0` |  
| `ConfigStarting` | 正通过 RPC 让 core 加载并启动某节点配置 | `neko_start_stage2()` 中 RPC Start 进行中 |  
| `ConfigRunning` | 某节点配置在 core 中运行，流量可走 | `started_id >= 0` |  
| `ConfigStopping` | 正在让 core 停掉当前配置（进程不动） | `neko_stop()` 中 RPC Stop 进行中 |  
| `Reconfiguring` | 配置参数变更（端口、路由等），需带当前节点重建配置 | 各处"改设置后调 `neko_start(started_id)`"的散落调用 |  
| `Stopping` | 要让整个 core 进程退出（非停配置） | `stop_core_daemon()` / 退出流程 |  
| `SpawnFailed` | 进程未拉起（二进制缺失、端口被占等） | `failed_to_start=true` + "failed to serve" 主动 kill 两条路径 |  
| `CrashBackoff` | 进程意外退出后的冷却等待期 | `setTimeout(1s)` + `restarting` + `coreRestartTimer` 频率限制 |  
| `PendingProfile` | "进程就绪后自动启动此节点"的挂起请求 | `start_profile_when_core_is_up` 字段 |  
  
### 转换说明  

**正常启动路径**  
  
- `Stopped → Spawning`：用户点启动，或程序启动时自动恢复上次节点。动作：拉起 `nekobox_core` 子进程。  
- `Spawning → GrpcReady`：子进程 stdout 出现 `"grpc server listening"`。重构后可换成端口探测/healthcheck，转换语义不变。  
- `GrpcReady → ConfigStarting`：发 `startProfile(id)` 意图（用户双击节点，或 `PendingProfile` 补发）。  
- `ConfigStarting → ConfigRunning`：RPC Start 成功。用户视角：节点"连上了"。  
  
**停配置（进程不死）**  
  
- `ConfigRunning → ConfigStopping`：用户点停止，或切换节点（先停旧的）。  
- `ConfigStopping → GrpcReady`：RPC Stop 完成或超时兜底。回到"进程活着但无配置"的空转态——现有代码中真实存在但未命名的状态。  
  
**改配置重启**  
  
- `ConfigRunning → Reconfiguring`：运行中改了影响 core 配置的内容（入站端口、路由模式、TUN inbound 开关等）。  
- `Reconfiguring → ConfigStarting`：重新走启动流程。场景：改设置后手动 restart 的每一处调用都是这条转换。  
  
**启动失败**  

- `Spawning → SpawnFailed`：两种触发——`FailedToStart`（进程未 exec 起来），或 stdout 出现 `"failed to serve"`（gRPC 监听失败，代码主动 kill）。单列此状态的原因：现有代码中两条路径重试策略不同，不拆开就会在 backoff 逻辑里写 `if (failed_to_start) return;` 补丁。  
- `SpawnFailed → Stopped`：报错、记录日志、回落（如提示端口被占）。  
  
**进程整体退出（正常）**  
  
- `GrpcReady / ConfigRunning / ConfigStopping → Stopping`：退出程序或显式 stop core。`ConfigRunning` 进 `Stopping` 隐含"先 RPC Stop 再退出进程"。  
- `Stopping → Stopped`：`QProcess::stateChanged(NotRunning)`，或 kill 超时兜底。  
  
**崩溃恢复**  
  
- `GrpcReady / ConfigRunning → CrashBackoff`：进程意外退出（NotRunning 且非 `prepare_exit`、非主动停止）。空转崩和跑配置崩都进此态。  
- `CrashBackoff → Spawning`：冷却结束（现行为 1 秒）且频率未超限，自动重启。  
- `CrashBackoff → Stopped`：10 秒内反复崩溃则放弃，打印 "Core exits too frequently"。现为 `coreRestartTimer` + `return` 的静默失败，建模后成为可见终态转移。  
  
**挂起请求**  
  
- `Spawning → PendingProfile`：用户在 core 未就绪时点启动。现行为把 id 存入 `start_profile_when_core_is_up` 后 `Restart()` 走人。状态图将其显式化为"延迟履行的请求"。  
- `PendingProfile → ConfigStarting`：`GrpcReady` 到达后自动补发启动。此链替代 `MW_dialog_message("CoreStarted,"+id)` 字符串协议。  
  
### 解决的问题  

1. **`core_running=false` 语义含糊**：它既表示"进程死了"也表示"进程在但 gRPC 未 ready"。`neko_start` 靠此 flag 决定是否重启——`Spawning` 期间用户再点会被误判并二次 Restart。有了 `Spawning` 态，第二次请求进 `PendingProfile` 排队。  
2. **崩溃重启窗口无保护**：`CrashBackoff` 期间用户请求行为未定义，建模后明确为入队等待。  
3. **退出序列确定化**：退出 = 驱动状态机走到 `Stopped`，替代 `mu_exit.tryLock` + 递归调 `on_menu_exit_triggered` 的重试 hack。  
  
---  

