#include "cfg_management/ConfigRegistry.h"

namespace app::cfg_management{

using namespace app::infra::config;

void ConfigRegistry::register_section(IConfigSection &sec)
    {
    sections_[std::string(sec.id())] = &sec;
    }

IConfigSection *ConfigRegistry::find(std::string_view id) noexcept
    {
    const auto it = sections_.find(std::string(id));
    return it == sections_.end() ? nullptr : it->second;
    }

const IConfigSection *ConfigRegistry::find(std::string_view id) const noexcept
    {
    const auto it = sections_.find(std::string(id));
    return it == sections_.end() ? nullptr : it->second;
    }

void ConfigRegistry::load_all(const IConfigStore &store)
    {
    for (auto &[id, sec] : sections_)
        sec->load_from(store);
    }

void ConfigRegistry::save_all(IConfigStore &store)
    {
    for (auto &[id, sec] : sections_)
        sec->save_to(store);
    }

void ConfigRegistry::save_missing(IConfigStore &store)
    {
    for (auto &[id, sec] : sections_)
        {
        if (store.exists(id))
            continue; // 用户已经有一份：保持原样，绝不动它
        // 首次运行：此时 sec 已完成 load_from，save_to 写出的是"当前有效值"
        // （用户配置不存在 -> load 来自模板；模板也不存在 ->
        // 来自该段内置默认）。
        sec->save_to(store);
        }
    }

std::vector<std::string_view> ConfigRegistry::ids() const
    {
    std::vector<std::string_view> out;
    out.reserve(sections_.size());
    for (const auto &[id, sec] : sections_)
        out.push_back(id);
    return out;
    }

} // namespace app::cfg_management
