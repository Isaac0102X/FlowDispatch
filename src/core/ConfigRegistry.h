// ConfigRegistry.h — 配置注册中心：只登记/查询/分发，不持有任何业务配置细节。
// 这是"开闭原则"的落点：新增业务 = 新增 ConfigSection +
// 注册一行，本类一行不改。
#pragma once

#include "infra/config/IConfigSection.h"
#include "infra/config/IConfigStore.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace app::core {

class ConfigRegistry
    {
    public:
        void register_section(app::infra::config::IConfigSection &sec);

        app::infra::config::IConfigSection *find(std::string_view id) noexcept;
        const app::infra::config::IConfigSection *
        find(std::string_view id) const noexcept;

        void load_all(const app::infra::config::IConfigStore &store);
        void save_all(app::infra::config::IConfigStore &store);

        std::vector<std::string_view> ids() const;

    private:
        std::map<std::string, app::infra::config::IConfigSection *, std::less<>>
            sections_;
    };

} // namespace app::core
