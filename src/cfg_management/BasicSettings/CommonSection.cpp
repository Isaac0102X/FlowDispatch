#include "cfg_management/BasicSettings/CommonSection.h"

#include "infra/logging/Log.h"

namespace app::cfg_management::BasicSettings{

using namespace app::infra::config;

void CommonSection::load_from(const IConfigStore &store)
    {
    const FieldMap m = store.load(id());
    data_.inbound_address =
        field_or(m, "inbound_address", data_.inbound_address);
    data_.inbound_auth = field_or(m, "inbound_auth", data_.inbound_auth);
    data_.custom_inbound =
        field_or(m, "custom_inbound", data_.custom_inbound);
    data_.inbound_socks_port =
        field_or(m, "inbound_socks_port", data_.inbound_socks_port);
    data_.test_latency_url =
        field_or(m, "test_latency_url", data_.test_latency_url);
    data_.test_concurrent =
        field_or(m, "test_concurrent", data_.test_concurrent);
    data_.test_download_url =
        field_or(m, "test_download_url", data_.test_download_url);
    data_.test_download_timeout =
        field_or(m, "test_download_timeout", data_.test_download_timeout);
    data_.check_include_pre =
        field_or(m, "check_include_pre", data_.check_include_pre);
    data_.old_share_link_format =
        field_or(m, "old_share_link_format", data_.old_share_link_format);
    data_.system_proxy_format =
        field_or(m, "system_proxy_format", data_.system_proxy_format);
    }

void CommonSection::save_to(IConfigStore &store) const
    {
    FieldMap m;
    m["inbound_address"] = data_.inbound_address;
    m["inbound_auth"] = data_.inbound_auth;
    m["custom_inbound"] = data_.custom_inbound;
    m["inbound_socks_port"] = data_.inbound_socks_port;
    m["test_latency_url"] = data_.test_latency_url;
    m["test_concurrent"] = data_.test_concurrent;
    m["test_download_url"] = data_.test_download_url;
    m["test_download_timeout"] = data_.test_download_timeout;
    m["check_include_pre"] = data_.check_include_pre;
    m["old_share_link_format"] = data_.old_share_link_format;
    m["system_proxy_format"] = data_.system_proxy_format;
    store.save(id(), m);
    }

std::vector<UiField> CommonSection::ui_fields() const
    {
    return {
        {"inbound_address", "监听地址", data_.inbound_address},
        {"inbound_auth", "入站认证", data_.inbound_auth},
        {"custom_inbound", "自定义入站", data_.custom_inbound},
        {"inbound_socks_port", "混合监听端口", data_.inbound_socks_port},
        {"test_latency_url", "延迟测试地址", data_.test_latency_url},
        {"test_concurrent", "并发测试数", data_.test_concurrent},
        {"test_download_url", "下载测试地址", data_.test_download_url},
        {"test_download_timeout", "下载测试超时(秒)",
         data_.test_download_timeout},
        {"check_include_pre", "包含预发布版本", data_.check_include_pre},
        {"old_share_link_format", "旧版分享链接格式",
         data_.old_share_link_format},
        {"system_proxy_format", "系统代理格式", data_.system_proxy_format},
    };
    }

void CommonSection::apply(const FieldMap &edits, IConfigStore &store)
    {
    data_.inbound_address =
        field_or(edits, "inbound_address", data_.inbound_address);
    data_.inbound_auth =
        field_or(edits, "inbound_auth", data_.inbound_auth);
    data_.custom_inbound =
        field_or(edits, "custom_inbound", data_.custom_inbound);
    data_.inbound_socks_port =
        field_or(edits, "inbound_socks_port", data_.inbound_socks_port);
    data_.test_latency_url =
        field_or(edits, "test_latency_url", data_.test_latency_url);
    data_.test_concurrent =
        field_or(edits, "test_concurrent", data_.test_concurrent);
    data_.test_download_url =
        field_or(edits, "test_download_url", data_.test_download_url);
    data_.test_download_timeout =
        field_or(edits, "test_download_timeout", data_.test_download_timeout);
    data_.check_include_pre =
        field_or(edits, "check_include_pre", data_.check_include_pre);
    data_.old_share_link_format =
        field_or(edits, "old_share_link_format", data_.old_share_link_format);
    data_.system_proxy_format =
        field_or(edits, "system_proxy_format", data_.system_proxy_format);
    save_to(store);
    APP_LOG_INFO("Common settings updated");
    notify_change("Common settings updated");
    }

} // namespace app::cfg_management
