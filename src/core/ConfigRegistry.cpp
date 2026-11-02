#include "core/ConfigRegistry.h"

namespace app::core{

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

std::vector<std::string_view> ConfigRegistry::ids() const
    {
    std::vector<std::string_view> out;
    out.reserve(sections_.size());
    for (const auto &[id, sec] : sections_)
        out.push_back(id);
    return out;
    }

} // namespace app::core
