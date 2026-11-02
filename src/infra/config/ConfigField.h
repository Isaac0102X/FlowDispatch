// ConfigField.h — 配置的"中立值类型"（业务逻辑与 UI 只见这里，绝不 include
// nlohmann/json）
//
// 设计说明（KISS）：本项目业务配置全部是标量字段（bool / int / string /
// double）， 因此用一个扁平字段表就足够，不需要递归 JSON 树。这样 std::variant
// 无递归、 实现极简，且彻底把 nlohmann 隔离在适配层（adapters/json）。
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

namespace app::infra::config{

// 单个标量字段值
struct FieldValue
    {
    using Storage =
        std::variant<std::nullptr_t, bool, std::int64_t, double, std::string>;
    Storage v{nullptr};

    FieldValue() = default;
    FieldValue(bool b) : v(b) {}
    FieldValue(std::int64_t i) : v(i) {}
    FieldValue(int i) : v(static_cast<std::int64_t>(i)) {} // 方便 int 字面量
    FieldValue(double d) : v(d) {}
    FieldValue(std::string s) : v(std::move(s)) {}
    FieldValue(const char *s) : v(std::string(s)) {}
    };

// 一个配置段的字段表（扁平对象）
using FieldMap = std::map<std::string, FieldValue, std::less<>>;

// UI 表单字段（给不同弹框生成编辑表单）
struct UiField
    {
    std::string key;
    std::string label;
    FieldValue value;
    };

// 从字段表安全读取：int / int64_t 统一从 int64_t 读；缺失或类型不符回退默认值。
// 业务配置段的 load/apply 用它，保持代码极简。
namespace detail{

// 把业务类型 T 映射到 variant 中实际的存储类型
template <typename T>
struct field_storage { using type = T; };

template <> struct field_storage<int>           { using type = std::int64_t; };
template <> struct field_storage<std::int64_t>  { using type = std::int64_t; };
// 其它（bool / double / std::string）默认就是自身，无需特化

template <typename T>
using field_storage_t = typename field_storage<T>::type;

}//namespace detail

template <typename T>
inline T field_or(const FieldMap &m, std::string_view key, T def)
    {
    const auto it = m.find(std::string(key));
    if (it == m.end())
        return def;

    // 一步完成：类型映射 -> get_if -> 命中即返回，否则回退默认值
    if (const auto *p = std::get_if<detail::field_storage_t<T>>(&it->second.v))
        return static_cast<T>(*p);
    return def;
    }
    
} // namespace app::infra::config
