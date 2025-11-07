#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

/**
 * Helper function to remove leading and trailing whitespace from a string
 * @param s The input string to trim
 * @return A new string with whitespace removed from both ends
 */
static inline std::string trim(const std::string &s) {
    size_t a = 0;
    while (a < s.size() && isspace((unsigned char)s[a])) ++a;
    size_t b = s.size();
    while (b > a && isspace((unsigned char)s[b-1])) --b;
    return s.substr(a, b-a);
}

/**
 * Loads and parses an INI-style configuration file
 * @param path The filesystem path to the configuration file
 * @param err Reference to string where error messages will be stored
 * @return true if file was successfully loaded and parsed, false otherwise
 */
bool Config::loadFromFile(const std::string &path, std::string &err) {
    // Try to open the configuration file
    std::ifstream ifs(path);
    if (!ifs) {
        err = "Failed to open config file: "+path;
        return false;
    }
    std::string line;      // Current line being processed
    std::string current;    // Current section name
    int lineno = 0;        // Line number for error reporting
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

/**
 * Checks if a section exists in the configuration
 * @param section The name of the section to check
 * @return true if the section exists, false otherwise
 */
bool Config::hasSection(const std::string &section) const {
    return data_.find(section) != data_.end();
}

/**
 * Retrieves a value from the configuration
 * @param section The section name containing the key
 * @param key The key whose value to retrieve
 * @param defaultValue Value to return if section or key doesn't exist
 * @return The value associated with the key, or defaultValue if not found
 */
std::string Config::get(const std::string &section, const std::string &key, const std::string &defaultValue) const {
    auto it = data_.find(section);
    if (it == data_.end()) return defaultValue;
    auto it2 = it->second.find(key);
    if (it2 == it->second.end()) return defaultValue;
    return it2->second;
}

/**
 * Returns a list of all section names in the configuration
 * @return Vector containing all section names
 */
std::vector<std::string> Config::sections() const {
    std::vector<std::string> out;
    out.reserve(data_.size());  // Pre-allocate space for efficiency
    for (auto &kv : data_) out.push_back(kv.first);
    return out;
}
