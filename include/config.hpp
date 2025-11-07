#pragma once
#include <string>
#include <unordered_map>

class Config {
public:
    Config() = default;

    // Load INI file. Returns false on failure and sets err.
    bool loadFromFile(const std::string &path, std::string &err);

    // Get value from [section] key, return default_val if not present.
    std::string get(const std::string &section, const std::string &key, const std::string &default_val = "") const;

private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> data_;
};