#include "server.h"
#include "auth.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main() {
    Server server(8080);
    
    // 设置静态文件目录
    server.serveStatic("public");
    
    // 设置会话认证
    SessionAuth* session_auth = new SessionAuth();
    server.useSessionAuth(session_auth);
    
    // 公共路由
    server.get("/api/public", [](Request& req, Response& res) {
        res.json("{\"message\": \"This is public data\", \"timestamp\": \"" + getCurrentTime() + "\"}");
    });
    
    // 登录页面
    server.get("/login", [](Request& req, Response& res) {
        std::ifstream file("public/login.html");
        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)), 
                               std::istreambuf_iterator<char>());
            res.html(content);
        } else {
            res.status(404).send("Login page not found");
        }
    });
    
    // 受保护的路由
    server.get("/api/protected", [](Request& req, Response& res) {
        res.json("{\"message\": \"This is protected data\", \"timestamp\": \"" + getCurrentTime() + "\"}");
    });
    
    // 动态路由
    server.get("/api/users/:id", [](Request& req, Response& res) {
        std::string userId = req.params["id"];
        res.json("{\"user_id\": \"" + userId + "\", \"name\": \"User " + userId + "\"}");
    });
    
    // API POST 示例
    server.post("/api/users", [](Request& req, Response& res) {
        // 这里应该解析JSON并创建用户
        res.status(201).json("{\"message\": \"User created\", \"id\": \"123\"}");
    });
    
    // 注销
    server.get("/logout", [&](Request& req, Response& res) {
        // 从Cookie中获取session_id
        if (req.headers.find("Cookie") != req.headers.end()) {
            std::string cookie_header = req.headers.at("Cookie");
            size_t session_pos = cookie_header.find("session_id=");
            
            if (session_pos != std::string::npos) {
                session_pos += 11; // Move past "session_id="
                size_t end_pos = cookie_header.find(';', session_pos);
                
                std::string session_id;
                if (end_pos == std::string::npos) {
                    session_id = cookie_header.substr(session_pos);
                } else {
                    session_id = cookie_header.substr(session_pos, end_pos - session_pos);
                }
                
                server.getSessionAuth()->destroySession(session_id);
            }
        }
        
        // 清除Cookie并重定向到登录页面
        res.setHeader("Set-Cookie", "session_id=; expires=Thu, 01 Jan 1970 00:00:00 GMT; Path=/")
           .status(302).setHeader("Location", "/login").send("");
    });
    
    std::cout << "Starting server on port 8080..." << std::endl;
    std::cout << "Open http://localhost:8080 in your browser" << std::endl;
    
    // 启动服务器
    server.run();
    
    return 0;
}