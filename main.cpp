#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include "httplib.h"
#include <sstream>
#include <iomanip>
#include <locale>
#include <codecvt>
#include "base64.hpp"

// Global configuration variables
std::string AUTH_TOKEN;
std::string LPAC_PATH;
int PORT = 5000; // default value
bool ENABLE_EXECUTE = false;

void loadConfig(const std::string &configPath) {
    std::ifstream configFile(configPath);
    if (!configFile) {
        std::cerr << "Could not open config file: " << configPath << std::endl;
        exit(1);
    }

    std::unordered_map<std::string, std::string> configMap;
    std::string line;
    while (std::getline(configFile, line)) {
        auto delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) continue;
        
        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);

        configMap[key] = value;
    }

    // Set global variables based on config
    if (configMap.count("port")) PORT = std::stoi(configMap["port"]);
    if (configMap.count("lpac_path")) LPAC_PATH = configMap["lpac_path"];
    if (configMap.count("enable_execute")) ENABLE_EXECUTE = (configMap["enable_execute"] == "true");
    if (configMap.count("auth_token")) AUTH_TOKEN = configMap["auth_token"];
}


/*
std::string base64_decode(const std::string &in) {
    const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) {
        T[base64_chars[i]] = i;
    }

    // 获取实际的字符串长度
    size_t in_len = in.length();
    
    // 预分配输出缓冲区
    out.reserve((in_len * 3) / 4);

    unsigned int val = 0;
    int valb = -8;
    
    // 使用显式长度而不是依赖字符串结束符
    for (size_t i = 0; i < in_len; i++) {
        unsigned char c = in[i];
        if (T[c] == -1) continue;
        
        val = (val << 6) + T[c];
        valb += 6;
        
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    std::cout << out;
    out = out + "\0";
    std::cout << out;
    return out;
}
*/

// 执行系统命令并返回输出
std::string execCommand(const std::string &command)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }

    return result;
}

int main(int argc, char *argv[])
{
    std::string configPath = "./config.txt"; // default path
    if (argc > 1 && std::string(argv[1]) == "-c" && argc > 2) {
        configPath = argv[2];
    }

    loadConfig(configPath);
    std::cout << "Server running on port: " << PORT << std::endl;
    std::cout << "LPAC Path: " << LPAC_PATH << std::endl;
    std::cout << "Execute Enabled: " << (ENABLE_EXECUTE ? "Yes" : "No") << std::endl;
    std::cout << "Auth Token: " << AUTH_TOKEN << std::endl;
    // std::locale::global(std::locale("en_US.UTF-8")); //linux
    // setlocale(LC_ALL, "en_US.UTF-8"); //win
    httplib::Server svr;

    // 定义 /execute 路由处理
    svr.Get(R"(/execute/(.*))", [](const httplib::Request &req, httplib::Response &res)
            {
        // 验证鉴权 Token
        auto auth = req.get_header_value("Authorization");
        if (auth != "Bearer " + AUTH_TOKEN) {
            res.status = 401;
            res.set_content("Unauthorized", "text/plain");
            return;
        }
        if (!ENABLE_EXECUTE){
            res.set_content("Execute not enabled.", "text/plain");
        }
        // 获取 /execute/ 后的参数
        std::string command_path = req.matches[1];
        if (command_path.empty()) {
            res.status = 400;
            res.set_content("Bad Request: Missing command path", "text/plain");
            return;
        }

        // 将斜杠替换为空格
        std::replace(command_path.begin(), command_path.end(), '/', ' ');

        // 构建 lpac 命令
        std::string command = LPAC_PATH + command_path;
        std::cout << "custom command: " << command;
        // 执行命令并获取输出
        try {
            std::string output = execCommand(command);
            res.set_content(output, "text/plain");
            std::cout << output;
        } catch (const std::exception &e) {
            res.status = 500;
            res.set_content(std::string("Command execution failed: ") + e.what(), "text/plain");
        } });

    // New endpoint for /getprofile
    svr.Get("/getprofile", [&](const httplib::Request &req, httplib::Response &res)
            {
        auto auth = req.get_header_value("Authorization");
        //std::cout << "/getprofile";
        if (auth != "Bearer " + AUTH_TOKEN) {
            res.status = 401;
            res.set_content("Unauthorized", "text/plain");
            return;
        }
        std::cout << "/getprofile";
        // Execute the lpac command
        std::string output = execCommand(LPAC_PATH + " profile list");
        std::cout << output;
        res.set_content(output, "text/plain"); });

    svr.Get("/chipinfo", [&](const httplib::Request &req, httplib::Response &res)
            {
        auto auth = req.get_header_value("Authorization");
        if (auth != "Bearer " + AUTH_TOKEN) {
            res.status = 401;
            res.set_content("Unauthorized", "text/plain");
            return;
        }
        std::cout << "/chipinfo";
        // Execute the lpac command
        std::string output = execCommand(LPAC_PATH + " chip info");
        std::cout << output;
        res.set_content(output, "text/plain"); });

    svr.Get(R"(/enableprofile/(\w+))", [&](const httplib::Request &req, httplib::Response &res)
            {
        auto auth = req.get_header_value("Authorization");
        if (auth != "Bearer " + AUTH_TOKEN) {
            res.status = 401;
            res.set_content("Unauthorized", "text/plain");
            return;
        }

        // Extract ICCID from URL
        std::string iccid = req.matches[1];
        if (iccid.length() <= 6 || !std::all_of(iccid.begin(), iccid.end(), ::isdigit)) {
            res.set_content("invalid iccid", "text/plain");
            return;
        }
        std::cout << "/enableprofile/" << iccid;
        // Construct the command with the ICCID
        std::string command = LPAC_PATH + " profile enable " + iccid;
        std::string output = execCommand(command.c_str());
        std::cout << output;

        // Return the command output
        res.set_content(output, "text/plain"); });

    svr.Get(R"(/disableprofile/(\w+))", [&](const httplib::Request &req, httplib::Response &res)
            {
        auto auth = req.get_header_value("Authorization");
        if (auth != "Bearer " + AUTH_TOKEN) {
            res.status = 401;
            res.set_content("Unauthorized", "text/plain");
            return;
        }

        // Extract ICCID from URL
        std::string iccid = req.matches[1];
        if (iccid.length() <= 6 || !std::all_of(iccid.begin(), iccid.end(), ::isdigit)) {
            res.set_content("invalid iccid", "text/plain");
            return;
        }
        std::cout << "/disableprofile/" << iccid;
        // Construct the command with the ICCID
        std::string command = LPAC_PATH + " profile disable " + iccid;
        std::string output = execCommand(command.c_str());
        std::cout << output;

        // Return the command output
        res.set_content(output, "text/plain"); });

    // 定义 /nickname/{iccid} 路由处理
    svr.Post(R"(/nickname/(\d+))", [](const httplib::Request &req, httplib::Response &res) {
        // 验证鉴权 Token
        auto auth = req.get_header_value("Authorization");
        if (auth != "Bearer " + AUTH_TOKEN) {
            res.status = 401;
            res.set_content("Unauthorized", "text/plain");
            return;
        }

        // 获取 ICCID 参数并验证
        std::string iccid = req.matches[1];
        if (iccid.size() <= 6 || !std::all_of(iccid.begin(), iccid.end(), ::isdigit)) {
            res.status = 400;
            res.set_content("Invalid ICCID", "text/plain");
            return;
        }

        // URL解码请求体内容
        std::string encoded_nickname = req.body + "\0";
        std::string nickname;
        try {
            nickname = base64::from_base64(encoded_nickname)+"\0";
            std::cout << nickname;
        } catch (const std::exception &e) {
            res.status = 400;
            res.set_content("Invalid Base64 encoding", "text/plain");
            return;
        }
        std::string command = LPAC_PATH + " profile nickname " + iccid + " " + nickname;

        // 执行命令
        try {
            std::string output = execCommand(command);  // execCommand 是您用来执行命令的函数
            res.set_content(output, "text/plain");
        } catch (const std::exception &e) {
            res.status = 500;
            res.set_content(std::string("Command execution failed: ") + e.what(), "text/plain");
        }
    });


    // 运行服务器
    //std::cout << "Server running on port 5000" << std::endl;
    svr.listen("0.0.0.0", PORT);

    return 0;
}
