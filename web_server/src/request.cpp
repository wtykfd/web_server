#include "request.h"
#include "utils.h"
#include <sstream>
#include <iostream>

Request::Request(const std::string& raw_request) {
    parse(raw_request);
}

void Request::parse(const std::string& raw_request) {
    std::vector<std::string> lines = split(raw_request, '\n');
    
    if (lines.empty()) return;
    
    // 解析请求行
    parseRequestLine(lines[0]);
    
    // 解析头部
    std::vector<std::string> header_lines;
    size_t i = 1;
    for (; i < lines.size() && !lines[i].empty(); i++) {
        header_lines.push_back(lines[i]);
    }
    parseHeaders(header_lines);
    
    // 解析查询参数
    parseQueryParams();
    
    // 解析请求体
    if (i < lines.size()) {
        body = lines[++i];
    }
}

void Request::parseRequestLine(const std::string& line) {
    std::vector<std::string> parts = split(line, ' ');
    if (parts.size() >= 3) {
        method = parts[0];
        
        // 分离路径和查询参数
        size_t query_pos = parts[1].find('?');
        if (query_pos != std::string::npos) {
            path = parts[1].substr(0, query_pos);
        } else {
            path = parts[1];
        }
        
        version = parts[2];
    }
}

void Request::parseHeaders(const std::vector<std::string>& header_lines) {
    for (const auto& line : header_lines) {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // 去除首尾空白字符
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            headers[key] = value;
        }
    }
}

void Request::parseQueryParams() {
    size_t query_pos = path.find('?');
    if (query_pos == std::string::npos) return;
    
    std::string query_str = path.substr(query_pos + 1);
    path = path.substr(0, query_pos);
    
    std::vector<std::string> pairs = split(query_str, '&');
    for (const auto& pair : pairs) {
        size_t equal_pos = pair.find('=');
        if (equal_pos != std::string::npos) {
            std::string key = urlDecode(pair.substr(0, equal_pos));
            std::string value = urlDecode(pair.substr(equal_pos + 1));
            query[key] = value;
        }
    }
}