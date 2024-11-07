#include "third_party/uWebSockets/src/App.h"
#include "json.hpp"
#include <iostream>
#include <cstdio>
#include <string>
#include <memory>
#include <unordered_map>

using json = nlohmann::json;
const std::string AUTH_PASSWORD = "your_password"; // 设置的正确密码

// 执行 lpac 命令并返回输出
std::string executeLpacCommand(const std::string& command) {
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("Failed to run lpac command");
    }
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    return result;
}

// 处理命令请求，转换成 lpac 命令格式并执行
json handleLpacRequest(const json& request) {
    std::string action = request["action"];
    std::string command = "./lpac " + action; // 假设 lpac 可执行文件在当前目录

    // 添加额外参数
    if (request.contains("params")) {
        for (const auto& param : request["params"]) {
            command += " " + param.get<std::string>();
        }
    }

    // 执行 lpac 命令并获取结果
    std::string output = executeLpacCommand(command);

    // 返回结果
    json response;
    response["type"] = "EXECUTE";
    response["result"] = output;
    return response;
}

// WebSocket 服务器主函数
int main() {
    std::unordered_map<void*, bool> authenticated_clients; // 记录已认证的客户端

    uWS::App().ws<json>("/*", {
        .open = [&authenticated_clients](auto* ws) {
            std::cout << "Client connected" << std::endl;
            authenticated_clients[ws] = false; // 初始化未认证

            // 发送认证请求
            json authRequest;
            authRequest["type"] = "AUTH";
            authRequest["message"] = "Please provide password";
            ws->send(authRequest.dump(), uWS::OpCode::TEXT);
        },
        .message = [&authenticated_clients](auto* ws, std::string_view message, uWS::OpCode opCode) {
            json request = json::parse(message);
            json response;

            // 认证阶段
            if (!authenticated_clients[ws]) {
                if (request.contains("type") && request["type"] == "AUTH" && request.contains("password")) {
                    if (request["password"] == AUTH_PASSWORD) {
                        authenticated_clients[ws] = true;
                        response["type"] = "AUTH";
                        response["status"] = "success";
                        response["message"] = "Authentication successful";
                    } else {
                        response["type"] = "AUTH";
                        response["status"] = "failure";
                        response["message"] = "Invalid password";
                        ws->send(response.dump(), opCode);
                        ws->close(); // 关闭连接
                        return;
                    }
                }
                ws->send(response.dump(), opCode);
                return;
            }

            // 已认证用户的命令处理
            if (request.contains("type") && request["type"] == "EXECUTE") {
                try {
                    response = handleLpacRequest(request);
                } catch (const std::exception& e) {
                    response["type"] = "ERROR";
                    response["message"] = e.what();
                }
            } else {
                response["type"] = "ERROR";
                response["message"] = "Unknown request type or missing parameters";
            }

            // 发送响应
            ws->send(response.dump(), opCode);
        },
        .close = [&authenticated_clients](auto* ws, int code, std::string_view message) {
            std::cout << "Client disconnected" << std::endl;
            authenticated_clients.erase(ws); // 移除已断开连接的客户端
        }
    }).listen(8080, [](auto* token) {
        if (token) {
            std::cout << "WebSocket server listening on port 8080" << std::endl;
        }
    }).run();
}