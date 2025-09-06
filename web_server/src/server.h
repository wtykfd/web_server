#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <netinet/in.h>
#include "request.h"
#include "response.h"
#include "auth.h"

class Server {
public:
    Server(int port = 8080);
    ~Server();
    
    void run();
    void stop();
    
    // 路由处理
    void get(const std::string& path, std::function<void(Request&, Response&)> handler);
    void post(const std::string& path, std::function<void(Request&, Response&)> handler);
    void put(const std::string& path, std::function<void(Request&, Response&)> handler);
    void del(const std::string& path, std::function<void(Request&, Response&)> handler);
    
    // 静态文件服务
    void serveStatic(const std::string& directory);
    
    // 设置认证中间件
    void useAuth(AuthMiddleware* auth);
    
    // 设置会话认证
    void useSessionAuth(SessionAuth* session_auth);
    
    // 获取会话认证
    SessionAuth* getSessionAuth() { return session_auth_; }
    
private:
    int port_;
    int server_fd_;
    bool running_;
    sockaddr_in address_;
    
    std::unordered_map<std::string, 
        std::unordered_map<std::string, 
            std::function<void(Request&, Response&)>>> routes_;
    
    std::string static_dir_;
    AuthMiddleware* auth_middleware_;
    SessionAuth* session_auth_;
    
    void handleClient(int client_socket);
    bool matchRoute(const std::string& route, const std::string& path, 
                   std::unordered_map<std::string, std::string>& params);
    void serveFile(Response& response, const std::string& file_path);
    void handleError(Response& response, int status_code, const std::string& message = "");
};

#endif