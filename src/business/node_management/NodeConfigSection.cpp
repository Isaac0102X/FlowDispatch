// NodeConfigSection.cpp — 业务代码只与"中立 FieldMap"交互，完全不碰 nlohmann。
#include "business/node_management/NodeConfigSection.h"

namespace app::business::node_management {

using namespace app::infra::config;

void NodeConfigSection::load_from(const IConfigStore &store)
    {
    const FieldMap m = store.load(id());
    data_.timeout = field_or(m, "timeout", data_.timeout);
    data_.sync_interval = field_or(m, "sync_interval", data_.sync_interval);
    data_.auto_sync = field_or(m, "auto_sync", data_.auto_sync);
    data_.endpoint = field_or(m, "endpoint", data_.endpoint);
    }

void NodeConfigSection::save_to(IConfigStore &store) const
    {
    FieldMap m;
    m["timeout"] = static_cast<std::int64_t>(data_.timeout);
    m["sync_interval"] = static_cast<std::int64_t>(data_.sync_interval);
    m["auto_sync"] = data_.auto_sync;
    m["endpoint"] = data_.endpoint;
    store.save(id(), m);
    }

std::vector<UiField> NodeConfigSection::ui_fields() const
    {
    return {
        {"timeout", "节点超时(秒)", static_cast<std::int64_t>(data_.timeout)},
        {"sync_interval", "同步间隔(秒)",
         static_cast<std::int64_t>(data_.sync_interval)},
        {"auto_sync", "自动同步", data_.auto_sync},
        {"endpoint", "节点地址", data_.endpoint},
    };
    }

void NodeConfigSection::apply(const FieldMap &edits, IConfigStore &store)
    {
    // 校验 + 更新强类型
    data_.timeout = field_or(edits, "timeout", data_.timeout);
    data_.sync_interval = field_or(edits, "sync_interval", data_.sync_interval);
    data_.auto_sync = field_or(edits, "auto_sync", data_.auto_sync);
    data_.endpoint = field_or(edits, "endpoint", data_.endpoint);
    // 写回并通知业务热更新
    save_to(store);
    notify_change("节点管理配置已更新");
    }

} // namespace app::business::node_management
