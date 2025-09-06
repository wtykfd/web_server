#ifndef AUTH_H
#define AUTH_H

#include "request.h"
#include <string>
#include <unordered_map>

class AuthMiddleware {
public:
    virtual bool authenticate(Request& request) = 0;
    virtual ~AuthMiddleware() {}
};

class BasicAuth : public AuthMiddleware {
public:
    BasicAuth(const std::string& realm = "Restricted Area");
    void addUser(const std::string& username, const std::string& password);
    bool authenticate(Request& request) override;
    
private:
    std::string realm_;
    std::unordered_map<std::string, std::string> users_;
};

class SessionAuth : public AuthMiddleware {
public:
    SessionAuth(const std::string& session_cookie = "session_id");
    std::string createSession(const std::string& username);
    bool validateSession(const std::string& session_id);
    void destroySession(const std::string& session_id);
    bool authenticate(Request& request) override;
    
private:
    std::string session_cookie_;
    std::unordered_map<std::string, std::string> sessions_; // session_id -> username
};

#endif