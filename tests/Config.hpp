// Simple INI-style config loader for site-specific MQTT settings
#pragma once

#include <string>
#include <map>
#include <vector>

class Config {
public:
    // Load an INI-like file. Sections are [name], keys are key=value.
    // Lines starting with # or ; are comments. Blank lines are ignored.
    bool loadFromFile(const std::string &path, std::string &err);

    // Check if section exists
    bool hasSection(const std::string &section) const;

    // Get value, returns defaultValue if missing
    std::string get(const std::string &section, const std::string &key, const std::string &defaultValue = "") const;

    // Return list of section names
    std::vector<std::string> sections() const;

private:
    std::map<std::string, std::map<std::string, std::string>> data_;
};
