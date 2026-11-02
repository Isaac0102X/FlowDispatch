
#include <iostream>
#include <memory>
#include <string>
#include <fstream>

#include <grpcpp/grpcpp.h>

#include "libcore.grpc.pb.h" // 由 protoc 生成
#include "libcore.pb.h"      // 由 protoc 生成

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using libcore::ErrorResp;
using libcore::LibcoreService;
using libcore::LoadConfigReq;

class LibcoreClient
    {
public:
    LibcoreClient(std::shared_ptr<Channel> channel, std::string token)
        : stub_(LibcoreService::NewStub(channel)), token_(std::move(token))
        {
        }

    // json_config: sing-box 的 JSON 配置字符串
    // 返回值：空字符串代表成功，否则是错误信息
    std::string Start(const std::string &json_config)
        {
        LoadConfigReq request;
        request.set_core_config(json_config);
        request.set_enable_nekoray_connections(false);
        // stats_outbounds 留空即可

        ErrorResp response;
        ClientContext context;
        // nekoray 的 token 鉴权，等价于手写客户端里
        // request.setRawHeader("nekoray_auth", nekoray_auth) 那一行
        context.AddMetadata("nekoray_auth", token_);

        Status status = stub_->Start(&context, request, &response);

        if (!status.ok())
            {
            return "grpc call failed: " + status.error_message();
            }
        return response.error(); // 空字符串表示 core 侧启动成功
        }

private:
    std::unique_ptr<LibcoreService::Stub> stub_;
    std::string token_;
    };

int main(int argc, char **argv)
    {
    if (argc < 3)
        {
        std::cerr << "usage: " << argv[0]
                  << " <core_port> <token> [json_config_file]\n";
        return 1;
        }

    std::string target = "127.0.0.1:" + std::string(argv[1]);
    std::string token = argv[2];

    // 一个最小可用的 sing-box JSON 配置，仅用于跑通 demo
    std::string json_config = R"({
        "log": {"level": "info"},
        "inbounds": [],
        "outbounds": [{"type": "direct", "tag": "direct"}]
    })";

    if (argc >= 4)
        {
        // 如果传了文件路径，就读文件内容作为 json_config
        std::ifstream f(argv[3]);
        json_config.assign((std::istreambuf_iterator<char>(f)),
                           std::istreambuf_iterator<char>());
        }

    LibcoreClient client(
        grpc::CreateChannel(target, grpc::InsecureChannelCredentials()), token);

    std::string err = client.Start(json_config);
    if (err.empty())
        {
        std::cout << "core started successfully\n";
        }
    else
        {
        std::cout << "core start failed: " << err << "\n";
        }

    return 0;
    }
