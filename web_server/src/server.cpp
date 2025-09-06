#include "server.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>
#include "utils.h"

Server::Server(int port) : port_(port), running_(false), 
                          auth_middleware_(nullptr), session_auth_(nullptr) {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address_.sin_family = AF_INET;
    address_.sin_addr.s_addr = INADDR_ANY;
    address_.sin_port = htons(port_);
    
    if (bind(server_fd_, (struct sockaddr*)&address_, sizeof(address_)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

Server::~Server() {
    stop();
    if (auth_middleware_) delete auth_middleware_;
    if (session_auth_) delete session_auth_;
}

void Server::run() {
    if (listen(server_fd_, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    running_ = true;
    std::cout << "Server running on port " << port_ << std::endl;
    
    while (running_) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            if (running_) {
                perror("accept");
            }
            continue;
        }
        
        // 使用线程处理客户端请求
        std::thread client_thread(&Server::handleClient, this, client_socket);
        client_thread.detach();
    }
}

void Server::stop() {
    running_ = false;
    close(server_fd_);
}

void Server::get(const std::string& path, std::function<void(Request&, Response&)> handler) {
    routes_[path]["GET"] = handler;
}

void Server::post(const std::string& path, std::function<void(Request&, Response&)> handler) {
    routes_[path]["POST"] = handler;
}

void Server::put(const std::string& path, std::function<void(Request&, Response&)> handler) {
    routes_[path]["PUT"] = handler;
}

void Server::del(const std::string& path, std::function<void(Request&, Response&)> handler) {
    routes_[path]["DELETE"] = handler;
}

void Server::serveStatic(const std::string& directory) {
    static_dir_ = directory;
}

void Server::useAuth(AuthMiddleware* auth) {
    auth_middleware_ = auth;
}

void Server::useSessionAuth(SessionAuth* session_auth) {
    session_auth_ = session_auth;
}

void Server::handleClient(int client_socket) {
    char buffer[10240] = {0}; // 10KB buffer
    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer));
    
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    
    Request request(std::string(buffer, bytes_read));
    Response response;
    
    try {
        // 认证中间件
        if (auth_middleware_ && !auth_middleware_->authenticate(request)) {
            response.status(401).send("Unauthorized");
            response.sendTo(client_socket);
            close(client_socket);
            return;
        }
        
        // 会话认证中间件
        if (session_auth_ && !session_auth_->authenticate(request)) {
            // 检查是否是登录请求
            if (request.path == "/login" && request.method == "POST") {
                // 解析表单数据
                std::unordered_map<std::string, std::string> form_data;
                std::vector<std::string> pairs = split(request.body, '&');
                
                for (const auto& pair : pairs) {
                    size_t equal_pos = pair.find('=');
                    if (equal_pos != std::string::npos) {
                        std::string key = urlDecode(pair.substr(0, equal_pos));
                        std::string value = urlDecode(pair.substr(equal_pos + 1));
                        form_data[key] = value;
                    }
                }
                
                // 检查凭据（这里应该查询数据库）
                if (form_data["username"] == "admin" && form_data["password"] == "password") {
                    std::string session_id = session_auth_->createSession(form_data["username"]);
                    response.setHeader("Set-Cookie", "session_id=" + session_id + "; Path=/; HttpOnly");
                    response.status(302).setHeader("Location", "/").send("");
                } else {
                    response.status(302).setHeader("Location", "/login?error=1").send("");
                }
                
                response.sendTo(client_socket);
                close(client_socket);
                return;
            }
            
            // 重定向到登录页面
            if (request.path != "/login") {
                response.status(302).setHeader("Location", "/login").send("");
                response.sendTo(client_socket);
                close(client_socket);
                return;
            }
        }
        
        // 尝试匹配路由
        bool route_matched = false;
        std::unordered_map<std::string, std::string> params;
        
        for (const auto& route_entry : routes_) {
            if (matchRoute(route_entry.first, request.path, params) && 
                route_entry.second.find(request.method) != route_entry.second.end()) {
                
                // 添加参数到请求对象
                for (const auto& param : params) {
                    request.params[param.first] = param.second;
                }
                
                route_entry.second.at(request.method)(request, response);
                route_matched = true;
                break;
            }
        }
        
        // 如果没有匹配的路由，尝试提供静态文件
        if (!route_matched && !static_dir_.empty()) {
            std::string file_path = static_dir_ + request.path;
            if (request.path == "/") file_path += "index.html";
            
            if (fileExists(file_path)) {
                serveFile(response, file_path);
            } else {
                // 返回404页面
                handleError(response, 404);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling request: " << e.what() << std::endl;
        handleError(response, 500, e.what());
    }
    
    response.sendTo(client_socket);
    close(client_socket);
}

bool Server::matchRoute(const std::string& route, const std::string& path, 
                       std::unordered_map<std::string, std::string>& params) {
    // 简单的路由匹配实现，支持:param格式
    std::vector<std::string> route_parts = split(route, '/');
    std::vector<std::string> path_parts = split(path, '/');
    
    if (route_parts.size() != path_parts.size()) return false;
    
    for (size_t i = 0; i < route_parts.size(); i++) {
        if (route_parts[i].empty()) continue;
        
        if (route_parts[i][0] == ':') {
            // 参数匹配
            std::string param_name = route_parts[i].substr(1);
            params[param_name] = path_parts[i];
        } else if (route_parts[i] != path_parts[i]) {
            return false;
        }
    }
    
    return true;
}

void Server::serveFile(Response& response, const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        handleError(response, 500, "Failed to open file");
        return;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), 
                       std::istreambuf_iterator<char>());
    
    // 根据文件扩展名设置Content-Type
    std::string ext = getFileExtension(file_path);
    std::string content_type = getMimeType(ext);
    
    response.setHeader("Content-Type", content_type).send(content);
}

void Server::handleError(Response& response, int status_code, const std::string& message) {
    response.status(status_code);
    
    std::string error_page = static_dir_ + "/" + std::to_string(status_code) + ".html";
    if (fileExists(error_page)) {
        serveFile(response, error_page);
    } else {
        response.send("<html><body><h1>" + std::to_string(status_code) + " " + 
                     response.getStatusCode() + "</h1><p>" + message + "</p></body></html>");
    }
}