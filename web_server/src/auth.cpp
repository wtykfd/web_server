#include "auth.h"
#include "utils.h"
#include <iostream>
#include <ctime>
#include <random>

BasicAuth::BasicAuth(const std::string& realm) : realm_(realm) {}

void BasicAuth::addUser(const std::string& username, const std::string& password) {
    users_[username] = password;
}

bool BasicAuth::authenticate(Request& request) {
    if (request.headers.find("Authorization") == request.headers.end()) {
        return false;
    }
    
    std::string auth_header = request.headers.at("Authorization");
    if (auth_header.find("Basic ") != 0) {
        return false;
    }
    
    std::string encoded = auth_header.substr(6);
    std::string decoded = base64Decode(encoded);
    
    size_t colon_pos = decoded.find(':');
    if (colon_pos == std::string::npos) {
        return false;
    }
    
    std::string username = decoded.substr(0, colon_pos);
    std::string password = decoded.substr(colon_pos + 1);
    
    if (users_.find(username) == users_.end() || users_[username] != password) {
        return false;
    }
    
    return true;
}

SessionAuth::SessionAuth(const std::string& session_cookie) : session_cookie_(session_cookie) {}

std::string SessionAuth::createSession(const std::string& username) {
    // 生成随机session ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    const char* hex_chars = "0123456789abcdef";
    std::string session_id;
    
    for (int i = 0; i < 32; ++i) {
        session_id += hex_chars[dis(gen)];
    }
    
    sessions_[session_id] = username;
    return session_id;
}

bool SessionAuth::validateSession(const std::string& session_id) {
    return sessions_.find(session_id) != sessions_.end();
}

void SessionAuth::destroySession(const std::string& session_id) {
    sessions_.erase(session_id);
}

bool SessionAuth::authenticate(Request& request) {
    if (request.headers.find("Cookie") == request.headers.end()) {
        return false;
    }
    
    std::string cookie_header = request.headers.at("Cookie");
    size_t session_pos = cookie_header.find(session_cookie_ + "=");
    
    if (session_pos == std::string::npos) {
        return false;
    }
    
    session_pos += session_cookie_.size() + 1; // Move past "session_id="
    size_t end_pos = cookie_header.find(';', session_pos);
    
    std::string session_id;
    if (end_pos == std::string::npos) {
        session_id = cookie_header.substr(session_pos);
    } else {
        session_id = cookie_header.substr(session_pos, end_pos - session_pos);
    }
    
    return validateSession(session_id);
}