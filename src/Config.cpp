#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

static inline std::string trim(const std::string &s) {
    size_t a = 0;
    while (a < s.size() && isspace((unsigned char)s[a])) ++a;
    size_t b = s.size();
    while (b > a && isspace((unsigned char)s[b-1])) --b;
    return s.substr(a, b-a);
}

bool Config::loadFromFile(const std::string &path, std::string &err) {
    std::ifstream ifs(path);
    if (!ifs) {
        err = "Failed to open config file: "+path;
        return false;
    }
    std::string line;
    std::string current;
    int lineno = 0;
    while (std::getline(ifs, line)) {
        ++lineno;
        auto s = trim(line);
        if (s.empty()) continue;
        if (s[0] == '#' || s[0] == ';') continue;
        if (s.front() == '[' && s.back() == ']') {
            current = trim(s.substr(1, s.size()-2));
            if (current.empty()) {
                err = "Empty section name at line "+std::to_string(lineno);
                return false;
            }
            data_[current];
            continue;
        }
        auto eq = s.find('=');
        if (eq == std::string::npos) {
            err = "Invalid line " + std::to_string(lineno) + ": missing '='";
            return false;
        }
        std::string key = trim(s.substr(0, eq));
        std::string val = trim(s.substr(eq+1));
        if (key.empty()) {
            err = "Empty key at line "+std::to_string(lineno);
            return false;
        }
        if (current.empty()) {
            err = "Key/value outside of section at line "+std::to_string(lineno);
            return false;
        }
        data_[current][key] = val;
    }
    return true;
}

bool Config::hasSection(const std::string &section) const {
    return data_.find(section) != data_.end();
}

std::string Config::get(const std::string &section, const std::string &key, const std::string &defaultValue) const {
    auto it = data_.find(section);
    if (it == data_.end()) return defaultValue;
    auto it2 = it->second.find(key);
    if (it2 == it->second.end()) return defaultValue;
    return it2->second;
}

std::vector<std::string> Config::sections() const {
    std::vector<std::string> out;
    out.reserve(data_.size());
    for (auto &kv : data_) out.push_back(kv.first);
    return out;
}
