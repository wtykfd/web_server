#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <unordered_map>

class Response {
public:
    Response();
    
    Response& status(int code);
    Response& setHeader(const std::string& key, const std::string& value);
    Response& send(const std::string& body);
    Response& json(const std::string& json);
    Response& html(const std::string& html);
    
    void sendTo(int socket);
    std::string toString();
    
    int getStatusCode() const { return status_code; }
    
private:
    int status_code;
    std::string status_message;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    
    void setStatusMessage();
};

#endif