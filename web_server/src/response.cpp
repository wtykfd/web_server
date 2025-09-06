#include "response.h"
#include "utils.h"
#include <sstream>
#include <iostream>

Response::Response() : status_code(200) {
    setStatusMessage();
    headers["Server"] = "C++ Web Server";
    headers["Date"] = getCurrentTime();
    headers["Connection"] = "close";
}

Response& Response::status(int code) {
    status_code = code;
    setStatusMessage();
    return *this;
}

Response& Response::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
    return *this;
}

Response& Response::send(const std::string& body) {
    this->body = body;
    headers["Content-Length"] = std::to_string(body.size());
    if (headers.find("Content-Type") == headers.end()) {
        headers["Content-Type"] = "text/plain";
    }
    return *this;
}

Response& Response::json(const std::string& json) {
    headers["Content-Type"] = "application/json";
    return send(json);
}

Response& Response::html(const std::string& html) {
    headers["Content-Type"] = "text/html";
    return send(html);
}

void Response::sendTo(int socket) {
    std::string response_str = toString();
    send(socket, response_str.c_str(), response_str.size(), 0);
}

std::string Response::toString() {
    std::stringstream ss;
    ss << "HTTP/1.1 " << status_code << " " << status_message << "\r\n";
    
    for (const auto& header : headers) {
        ss << header.first << ": " << header.second << "\r\n";
    }
    
    ss << "\r\n" << body;
    
    return ss.str();
}

void Response::setStatusMessage() {
    switch (status_code) {
        case 200: status_message = "OK"; break;
        case 201: status_message = "Created"; break;
        case 400: status_message = "Bad Request"; break;
        case 401: status_message = "Unauthorized"; break;
        case 403: status_message = "Forbidden"; break;
        case 404: status_message = "Not Found"; break;
        case 500: status_message = "Internal Server Error"; break;
        default: status_message = "Unknown"; break;
    }
}