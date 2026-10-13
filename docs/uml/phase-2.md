# FlowDispatch 项目状态快照（project-state.md）

> 用途：作为新 AI session 的项目上下文基线，继续 FlowDispatch 的 UML
> 分析与设计。\
> 快照日期：2026-10-04\
> 原则：区分已确认结论与待验证事项；不把推导过程、废弃方案或未经确认的设计当作最终结论。

------------------------------------------------------------------------

## 1. 项目背景与目标

正在分析 GitHub 上的 NekoRay / nekobox
项目，并借鉴其架构与功能设计，开发自己的代理客户端项目
**FlowDispatch**。

已知的 NekoRay 架构背景：

-   `nekobox.exe` 是基于 Qt 的 UI 管理程序。
-   `core.exe` 是在 sing-box 基础上适配封装的 core 程序。
-   UI 与 core 通过 gRPC 通信，以支持功能扩展和配置管理。
-   TUN 模式涉及单 core、双 core 两种实现路径，以及 Linux / Windows
    的权限差异。

FlowDispatch 当前工作重点是通过 UML
完成从需求分析到设计的推导，并将设计映射到 C++
实现。当前讨论集中在订阅节点更新、职责分配、设计时序图和设计类模型的收敛。

## 2. 已完成的 UML 推导阶段

整体流程：

1.  编写用例文本（Use Case）
2.  绘制系统顺序图（System Sequence Diagram, SSD）
3.  建立领域模型（Domain Model）
4.  编写操作契约（Operation Contracts）
5.  将后置条件分配给最合适的对象（GRASP Responsibility Assignment）
6.  使用设计时序图（Design Sequence Diagram）验证职责分配
7.  整理设计类图（Design Class Diagram）

目前正在从第 6、7 步继续完善，并为第一轮实现确定边界。

## 3. 已确认的设计原则与决策

### 3.1 订阅与 Node 集合的职责边界

-   `Subscription` 负责表达"哪些 Node
    属于这个订阅"以及维护这层语义关系。
-   `SubscriptionRepository` 负责订阅聚合的持久化。
-   `NodeRepository` 负责全局 Node 的查找、添加等仓储职责。
-   `Subscription` 不直接依赖
    `NodeRepository`，避免把"有哪些节点"与"节点如何存储"耦合起来。

### 3.2 订阅节点更新：第一轮采用简单实现

已确认的迭代策略：

-   首轮不实现通用集合 Diff / identity matching。
-   首轮采用较简单的清空重建方式，以尽快跑通主流程。
-   但必须保护当前正在使用的活动节点，不能在更新订阅时将其删除，避免运行中的
    core 指向已失效对象。
-   当前活动节点由运行时对象 `ProxyCore`
    提供；更新流程取得该节点后，将其作为受保护节点交给 `Subscription`
    的节点更新操作。
-   未来迭代再完善
    `reconcileNodes()`，实现节点身份匹配、更新已有对象、新建新增对象、解除缺失对象关系等逻辑。

首轮接口讨论过
`replaceNodes(newNodes, protectedNode)`，但最终具体方法名、参数类型和节点所有权语义仍需在实现前确认。不能把某一种
C++ 指针形式视为已定稿。

关于未来迭代：

-   `replace` 与 `reconcile`
    表达不同语义，建议不要仅通过同名函数重载、悄悄改变实现含义。
-   可以保留首轮替换语义，未来增加明确的 `reconcileNodes()`。
-   是否采用函数重载作为演进手段尚未最终确定。

### 3.3 订阅拉取策略

已确认：

-   代理降级拉取不能由 `SubscriptionFetcher`
    在检测到网络失败后无条件自动触发。
-   是否允许代理降级由订阅配置决定，属性为
    `Subscription.allowProxyFallback`。
-   若历史状态表明订阅必须走代理，则直接代理拉取。
-   否则先尝试直连；直连失败后，仅在 `allowProxyFallback`
    为真时代理重试。
-   代理重试成功后，更新订阅领域状态，记录
    `proxyRequired`，使下次直接走代理。
-   不允许降级时，直连失败应向上报告错误。

协作逻辑示意（非最终 C++ API）：

``` cpp
RawData SubscriptionFetcher::fetch(Subscription& sub) {
    if (sub.isProxyRequired()) {
        return fetchViaProxy(sub.getUrl());
    }

    try {
        return fetchDirect(sub.getUrl());
    } catch (const NetworkException& e) {
        if (sub.allowProxyFallback()) {
            RawData data = fetchViaProxy(sub.getUrl());
            sub.markProxyRequired();
            return data;
        }
        throw e;
    }
}
```

### 3.4 ProxyCore、运行模式与 C++ 类型建模

已确认的方向：

-   `ProxyCore` 不负责操作系统系统代理的开关。
-   系统代理模式只需将 OS 代理设置指向本地
    `127.0.0.1:inbound_socks_port`；core 本身不感知"系统代理是否开启"。
-   `ProxyCore` 不再通过
    `enableSystemProxyMode()`、`enableTunMode()`、`disableRunMode()`
    直接承担所有模式操作。
-   `RunMode` 表达当前模式及其所需状态，不再设计成带
    `enable()/disable()/restore()` 的抽象基类。
-   当前讨论倾向于在 C++ 中使用封闭的 `std::variant`
    表达三种互斥模式：`NormalMode`、`SystemProxyMode`、`TunMode`。
-   通过 `std::visit` 分派具体操作；UML 中表达语义上的封闭变体，不强行把
    C++ 语法等同于 UML 继承。
-   不为简单的模式操作额外创建 Strategy 类；`std::function` +
    lambda、`variant` + `visit`、模板 + Concepts/Policy
    的取舍应结合实现需求，不应为了展示技术而增加抽象。
-   `TunTopology` 暂定为枚举，包含
    `SingleCore`、`DualCore`，不先创建两套 Strategy 类。

### 3.5 TUN 模式背景与复杂度拆分

NekoRay 的 TUN 机制背景：

-   **单 core 模式**：修改 core 配置后重新加载/运行；Linux 可通过
    `pkexec` 设置 `cap_net_admin=ep`，Windows
    则可能要求以管理员身份重启程序。
-   **双 core 模式**：在已有 core 外启动一个独立、提权的
    core，使用独立配置接收入站流量并将出站流量转发给旧 core。
-   双 core 可将 UI 与管理员权限隔离，但增加进程、转发链路和清理复杂度。
-   单 core / 双 core、Linux /
    Windows、最小权限原则同时兼顾，会显著增加设计复杂度。

FlowDispatch 的设计方向是将操作流程放在控制器中，并通过 Gateway
封装进程、OS 网络设置和 TUN
驱动操作。具体跨平台执行细节仍需通过后续设计时序图和实现验证。

### 3.6 恢复与残留状态

已确认的设计意图：

-   不保留 `RunMode.restore()` 作为公开多态操作。
-   启动时的残留检测与修复应拆成两个系统操作：检测、修复。
-   恢复系统代理时不能简单地"清空代理"，因为用户原有设置可能不是无代理状态。
-   OS 代理操作需要考虑捕获原始设置、设置代理、恢复原始快照。
-   残留状态需要有持久化机制；此前将相关职责命名为
    `ResidualConfigStore`，包含
    `save()`、`load()`、`clear()`。命名与具体保存内容仍需验证。
-   TUN 的路由恢复、适配器注销及失败清理需要在后续时序图中确认完整性。

## 4. 当前设计类清单（工作基线）

以下是目前讨论形成的工作清单，不代表所有签名与关系都已最终验证。

### 4.1 Controller

  类                          当前职责 / 操作
  --------------------------- ------------------------------------------
  `ScheduledTaskController`   管理计划任务设置与定时器注册/注销
  `NodeController`            导入节点、更新订阅
  `ProxyRuntimeController`    启停 core、切换系统代理与 TUN 等运行流程
  `MonitorController`         汇总流量统计、记录连接日志
  `StartupController`         启动时检测残留配置并执行修复流程

`RunModeController` 不单独保留；运行模式相关流程归入
`ProxyRuntimeController`。该控制器是否承担过多职责，列为 TODO。

### 4.2 Domain

  --------------------------------------------------------------------------------------------------------------------------------------------
  类 / 类型                                                   当前职责 / 操作
  ----------------------------------------------------------- --------------------------------------------------------------------------------
  `Node`（及协议子类）                                        节点领域信息；`isValidForCore()`

  `VmessNode`、`VlessNode`、`ShadowsocksNode`、`TrojanNode`   各自解析对应 URI 的 `fromUri()`

  `Subscription`                                              订阅信息、节点集合关系、`replaceNodes(...)`（首轮方案）、`markProxyRequired()`

  `ScheduledTask`                                             计划任务配置，如 `configure(...)`

  `RoutingRule`                                               路由规则领域信息

  `ProxyCore`                                                 core 运行状态、监听端口、当前活动节点、运行模式、规则及流量/连接记录相关状态

  `ConnectionLog`                                             连接日志的只读属性数据

  `RunMode`                                                   语义上的封闭模式类型；C++ 计划映射为 `std::variant`

  `NormalMode`                                                普通运行模式

  `SystemProxyMode`                                           系统代理模式及必要的原始设置快照

  `TunMode`                                                   TUN 模式、拓扑及必要快照

  `SourceType`                                                节点来源类型

  `TunTopology`                                               `SingleCore` / `DualCore`
  --------------------------------------------------------------------------------------------------------------------------------------------

此前类图将 `ProxyCore.activeNode` 作为当前活动节点关系，并据此删除
`Node.isActiveNode` 及
`NodeRepository.setActiveNode()/clearActiveNode()`，以避免重复保存活动状态。但活动节点的权威归属、引用生命周期与订阅节点集合之间的所有权关系仍需进一步核实。

### 4.3 Pure Fabrication / Infrastructure

  ------------------------------------------------------------------------------------------------------------------------------------------
  类                                  当前讨论的方法
  ----------------------------------- ------------------------------------------------------------------------------------------------------
  `NodeRepository`                    `add()`、`findById()`

  `SubscriptionRepository`            `findById()`、`save()`

  `ScheduledTaskRepository`           `saveOrUpdate()`、`persist()`

  `RoutingRuleRepository`             `findEffectiveRules()`

  `ConnectionLogStore`                `append()`、`query()`

  `NodeLinkParser`                    `parse()`

  `SubscriptionFetcher`               `fetch()`

  `CoreConfigBuilder`                 `build(...)`

  `CoreProcessGateway`                `isPortAvailable()`、`launch(config, privilegeLevel)`、`reload()`、`terminate()`、`waitUntilReady()`

  `OSNetworkGateway`                  `captureProxySettings()`、`setProxy()`、`restoreProxy(snapshot)`

  `TunDriverGateway`                  `createAdapter()`、`redirectDefaultRouting()`、`restoreDefaultRouting()`、`unregisterAdapter()`

  `TimerGateway`                      `registerWakeup()`、`unregisterWakeup()`

  `ResidualConfigStore`               `save()`、`load()`、`clear()`
  ------------------------------------------------------------------------------------------------------------------------------------------

技术/值类型候选：

-   `ProxySettingsSnapshot`
-   `TunSnapshot`
-   `PrivilegeLevel`：`Normal` / `Elevated`

这些类型及接口的精确字段、返回值、异常模型尚未定稿。

## 5. 关键协作流程成果图

以下只保留用于新 session
衔接的成果图。它们是当前工作基线；未验证的细节在图后列为 TODO。

### 5.1 Contract 3：更新订阅（首轮清空重建方案）

``` plantuml
@startuml Contract3_UpdateSubscription_V1
title Contract 3 - Update Subscription (V1)

actor User
participant ":System" as System
participant "NodeController" as Controller
participant "SubscriptionRepository" as SubRepo
participant "SubscriptionFetcher" as Fetcher
participant "NodeLinkParser" as Parser
participant "Subscription" as Sub
participant "ProxyCore" as Core

User -> System : updateSubscription(subscriptionId)
System -> Controller : updateSubscription(subscriptionId)

Controller -> SubRepo : findById(subscriptionId)
SubRepo --> Controller : sub

Controller -> Fetcher : fetch(sub)
alt sub.isProxyRequired()
    Fetcher -> Fetcher : fetchViaProxy(sub.url)
else
    Fetcher -> Fetcher : fetchDirect(sub.url)
    alt direct succeeds
        Fetcher --> Controller : RawData
    else direct fails
        alt sub.allowProxyFallback()
            Fetcher -> Fetcher : fetchViaProxy(sub.url)
            Fetcher -> Sub : markProxyRequired()
            Fetcher --> Controller : RawData
        else fallback disabled
            Fetcher --> Controller : NetworkException
        end
    end
end

alt fetch failed
    Controller --> System : updateFailed
    System --> User : show error
else fetch succeeded
    Controller -> Parser : parse(rawData)
    Parser --> Controller : newNodes

    Controller -> Core : getActiveNode()
    Core --> Controller : activeNode / null

    Controller -> Sub : replaceNodes(newNodes, protectedNode)
    note right of Sub
      V1 policy:
      - Preserve the protected active node
        if it belongs to this subscription.
      - Clear/replace other old nodes.
      - Add fetched nodes.
      - No general collection diff.
    end note
    Sub --> Controller : nodesReplaced

    Controller -> SubRepo : save(sub)
    SubRepo --> Controller : saved
    Controller --> System : success
    System --> User : refresh node list
end

@enduml
```

说明：

-   这是首轮订阅更新的协作草图。
-   活动节点由 `ProxyCore` 查询，保护判断由 `Subscription`
    在更新自身节点集合时执行。
-   `SubscriptionRepository` 保存更新后的聚合。
-   `NodeRepository` 不参与此流程。
-   `replaceNodes(newNodes, protectedNode)`
    是候选接口表达，具体签名未锁定。
-   图中未展开网络错误、解析错误、持久化失败时的事务回滚与恢复行为。

### 5.2 运行模式与 TUN 的设计约束摘要

C++ 映射示意（非最终代码）：

``` cpp
struct NormalMode {};

struct SystemProxyMode {
    ProxySettingsSnapshot originalSettings;
};

struct TunMode {
    TunTopology topology;
    TunSnapshot snapshot;
};

using RunMode = std::variant<
    NormalMode,
    SystemProxyMode,
    TunMode
>;
```

TUN 拓扑摘要：

``` plantuml
@startuml TunTopology
title TUN Topology

enum TunTopology {
  SingleCore
  DualCore
}

note right of TunTopology
  SingleCore:
  - 修改主 core 配置
  - 处理所需权限
  - 重新加载/重启相关运行配置

  DualCore:
  - 启动独立提权 core
  - 独立配置接收入站流量
  - 出站转发至旧 core
  - 关闭时需处理进程清理
end note
@enduml
```

此图只表达拓扑选项和已知行为，不是完整的 TUN
开关时序图。具体开关流程仍需在后续设计中验证。

## 6. 已否定或不采用的方案

-   `Subscription` 直接调用 `NodeRepository` 来更新节点集合。
-   首轮就实现通用节点集合 Diff / identity matching。
-   `SubscriptionFetcher` 无视订阅配置、自动代理降级。
-   `RunMode` 作为具有 `enable()`、`disable()`、`restore()`
    的抽象基类，并让每种模式承担所有开关和恢复行为。
-   `ProxyCore` 直接实现系统代理开关。
-   为运行模式或 TUN 拓扑仅仅为了套用模式模式而创建一套 Strategy 类。
-   将用户原有系统代理设置简单清空，而不保存/恢复原状态。
-   将残留检测与残留修复合并成一个不可区分的系统操作。

## 7. TODO：尚未确认或需要继续验证

### 7.1 订阅更新与对象生命周期

-   [ ] 明确 `Subscription` 节点集合与全局 `NodeRepository`
    的对象身份、所有权和持久化关系。
-   [ ] 明确当前活动节点是否可能属于订阅之外（例如手动导入节点）。
-   [ ] 明确活动节点不属于当前订阅时的保护行为。
-   [ ]
    明确订阅更新后，受保护节点若不在新数据中，是否继续保留在订阅集合、转为独立节点，或采用其他语义。
-   [ ] 明确清空重建时旧节点的删除/解除关联与新节点插入顺序。
-   [ ] 明确 `replaceNodes()`
    的参数类型、返回值、失败语义及异常安全保证。
-   [ ] 明确 `markProxyRequired()`
    在抓取成功但后续解析或保存失败时是否仍应持久化。
-   [ ]
    设计更新失败时的事务边界，避免当前连接或持久化数据处于不一致状态。
-   [ ] 后续迭代设计 `reconcileNodes()` 的身份匹配规则与增量更新策略。

### 7.2 ProxyCore 与 RunMode

-   [ ] 明确 `ProxyCore` 是否应保存完整
    `RunMode`，还是运行模式状态由`ProxyRuntimeController`/独立`RunMode`对象持有。
-   [ ] 明确 `ProxyCore.activeNode`
    的引用生命周期，避免订阅更新或仓储变更导致悬空引用。
-   [ ] 明确模式快照的创建、更新、清除时机，比如系统代理模式下需要先创建上一次的系统代理快照。
-   [ ] 明确普通模式、系统代理模式、TUN
    模式之间允许的状态转换及失败回滚。
-   [ ] 复核 `ProxyRuntimeController` 是否职责过重。

### 7.3 TUN、权限与恢复

-   [ ] 继续画并验证 Contract 5/6 的完整设计时序图。
-   [ ] 明确单 core 模式在 Linux 与 Windows 的权限/重启流程。
-   [ ] 明确双 core
    启停、就绪检测、终止失败、残留进程清理和重复开关请求的处理。
-   [ ] 明确 TUN 路由快照内容、捕获时机及恢复顺序。
-   [ ] 明确 TUN adapter 创建失败、路由重定向失败、core
    启动失败时的补偿动作。
-   [ ] 明确残留配置存储格式、版本兼容和崩溃恢复协议。
-   [ ] 确认 `CoreProcessGateway.reload()` 是否覆盖实际 core
    能力，或需区分 reload 与 restart。

### 7.4 设计模型与实现映射

-   [ ] 检查 `Node` 子类（VMess,Vless,Shadowsocks...）的扩展需求，以及 `fromUrl()` 是否应属于子类或由
    parser/factory 创建。
-   [ ] 明确 `SourceType` 是节点属性、导入上下文，还是两者之一。
-   [ ] 明确 `RoutingRule` 的聚合归属及有效规则合并语义。
-   [ ] 明确 `ConnectionLog` 的生成职责与 `ConnectionLogStore`
    的持久化边界。
-   [ ] 细化方法参数、返回值、可见性、异常和并发约束。
-   [ ] 在职责与协作关系稳定后，再决定 C++17/20/23 的具体实现技术。

## 8. 新 session 的建议起点

请将本文件视为 FlowDispatch 的上下文基线，而不是不可修改的最终规格。

建议继续顺序：

1.  审查 Contract 3
    订阅更新时序图，重点确认活动节点保护与节点所有权/生命周期是否自洽。
2.  补齐订阅更新失败路径：抓取失败、解析失败、替换失败、保存失败分别如何保持状态一致。
3.  回到 `ProxyRuntimeController`，继续验证 Contract 4（start/stop
    Proxy）与 Contract 5/6（RunMode/TUN）的协作和失败补偿。
4.  依据验证结果更新设计类图，而不是把当前工作清单视为完全定稿。

新 session 应优先遵守本文件明确标记的已确认决策；遇到 TODO
时先讨论和验证，不要自行补全为既定事实。
