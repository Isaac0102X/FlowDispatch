// NodeConfigSection.h — node_management 业务包自持的配置段。
// 头文件不 include 任何 json 库；业务代码面对强类型 NodeConfig。
#pragma once

#include "infra/config/IConfigSection.h"

#include <string>
#include <string_view>
#include <vector>

namespace app::business::node_management {

// 业务强类型配置
struct NodeConfig
    {
    int timeout = 30;
    int sync_interval = 5;
    bool auto_sync = false;
    std::string endpoint = "localhost:8080";
    };

class NodeConfigSection final : public app::infra::config::IConfigSection
    {
public:
    std::string_view id() const noexcept override
        {
        return "node_management/node_config";
        }

    void load_from(const app::infra::config::IConfigStore &store) override;
    void save_to(app::infra::config::IConfigStore &store) const override;

    std::vector<app::infra::config::UiField> ui_fields() const override;
    void apply(const app::infra::config::FieldMap &edits,
               app::infra::config::IConfigStore &store) override;

    NodeConfig &data() noexcept { return data_; }
    const NodeConfig &data() const noexcept { return data_; }

private:
    NodeConfig data_;
    };

} // namespace app::business::node_management
