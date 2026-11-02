// IConfigStore.h — 配置存储抽象（适配器目标）。
// 业务代码只面对 FieldMap，不知道也不关心后端是 json / ini / sqlite。
#pragma once

#include "infra/config/ConfigField.h"

#include <string_view>

namespace app::infra::config {

class IConfigStore
    {
    public:
        virtual ~IConfigStore() = default;

        // 按配置段 id 读取整段字段表（缺失返回空表）
        virtual FieldMap load(std::string_view section) const = 0;
        // 按配置段 id 整体写回
        virtual void save(std::string_view section, const FieldMap &fields) = 0;
        // 落盘（如需立即持久化）
        virtual void flush() = 0;
    };

} // namespace app::infra::config
