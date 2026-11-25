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

    // 读取一段配置（已在实现层完成"模板 + 用户"合并；业务只见最终字段表）
    virtual FieldMap load(std::string_view section) const = 0;

    // 整体写回本段字段（实现层负责保留未知字段 + 原子写）
    virtual void save(std::string_view section, const FieldMap &fields) = 0;

    // 用户配置文件是否已存在（出厂模板不算）
    // 用途：首次运行时生成一份完整、可编辑的用户配置；已存在的文件绝不覆盖（升级安全的核心）。
    virtual bool exists(std::string_view section) const = 0;

    // 落盘（如需立即持久化；目录式存储的 save 已即时落盘，故默认空实现）
    virtual void flush() = 0;
    };

} // namespace app::infra::config
