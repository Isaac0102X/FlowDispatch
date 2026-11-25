// CommonSection.h — CommonSection 选项卡基础设置段。文件：assets/config/basic/common.json
#pragma once

#include "infra/config/IConfigSection.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace app::cfg_management::BasicSettings{

struct CommonSettings
    {
    std::string inbound_address = "127.0.0.1";
    std::string inbound_auth = "{}";
    std::string custom_inbound = "{\"inbounds\": []}";
    std::int64_t inbound_socks_port = 2080;
    std::string test_latency_url = "http://cp.cloudflare.com/";
    std::int64_t test_concurrent = 5;
    std::string test_download_url =
        "http://cachefly.cachefly.net/10mb.test";
    std::int64_t test_download_timeout = 30;
    bool check_include_pre = false;
    bool old_share_link_format = false;
    std::string system_proxy_format = "env";
    };

class CommonSection final : public app::infra::config::IConfigSection
    {
public:
    std::string_view id() const noexcept override { return "basic/common"; }

    void load_from(const app::infra::config::IConfigStore &store) override;
    void save_to(app::infra::config::IConfigStore &store) const override;
    std::vector<app::infra::config::UiField> ui_fields() const override;
    void apply(const app::infra::config::FieldMap &edits,
               app::infra::config::IConfigStore &store) override;

    CommonSettings &data() noexcept { return data_; }
    const CommonSettings &data() const noexcept { return data_; }

private:
    CommonSettings data_;
    };

} // namespace app::cfg_management
