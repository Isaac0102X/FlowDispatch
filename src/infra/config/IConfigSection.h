// IConfigSection.h — 配置段抽象。每个业务包实现一个段，向 ConfigRegistry
// 自注册。 满足"开闭原则"：新增业务 = 新增段 + 注册一行，不改 core / ui /
// infra。
#pragma once

#include "infra/config/ConfigField.h"
#include "infra/config/IConfigStore.h"

#include <functional>
#include <string_view>
#include <vector>

namespace app::infra::config{

class IConfigSection
    {
    public:
        virtual ~IConfigSection() = default;

        virtual std::string_view id() const noexcept = 0;

        // 从 store 载入本段 → 反序列化为业务强类型
        virtual void load_from(const IConfigStore &store) = 0;
        // 业务强类型 → 写回 store
        virtual void save_to(IConfigStore &store) const = 0;

        // 给 UI 弹框的表单；不同弹框按 id 取不同段，触发点彼此独立
        virtual std::vector<UiField> ui_fields() const = 0;
        // 用户确认后：校验 → 更新强类型 → 持久化 → 通知业务热更新
        virtual void apply(const FieldMap &edits, IConfigStore &store) = 0;

        using ChangeCallback = std::function<void(std::string_view)>;
        void subscribe(ChangeCallback cb) { on_change_ = std::move(cb); }

    protected:
        void notify_change(std::string_view message) const
            {
            if (on_change_)
                on_change_(message);
            }

    private:
        ChangeCallback on_change_;
    };

} // namespace app::infra::config
