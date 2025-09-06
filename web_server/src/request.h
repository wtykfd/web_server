#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <unordered_map>
#include <vector>

class Request {
public:
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> params;
    std::unordered_map<std::string, std::string> query;
    std::string body;
    
    Request(const std::string& raw_request);
    void parse(const std::string& raw_request);
    
private:
    void parseRequestLine(const std::string& line);
    void parseHeaders(const std::vector<std::string>& header_lines);
    void parseQueryParams();
};

#endif