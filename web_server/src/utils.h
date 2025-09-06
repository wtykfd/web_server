#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

// 工具函数声明
std::vector<std::string> split(const std::string& s, char delimiter);
bool fileExists(const std::string& path);
std::string getFileExtension(const std::string& path);
std::string base64Encode(const std::string& input);
std::string base64Decode(const std::string& input);
std::string urlDecode(const std::string& value);
std::string getMimeType(const std::string& extension);
std::string getCurrentTime();

#endif