// Config implementation - simple INI-style parser.
//
// - loadFromFile(path, err):
//     Reads an INI-like file where sections are [section] and keys are key=value.
//     Lines starting with ';' or '#' are treated as comments and ignored.
//     Inline comments after a value (starting with ';' or '#') are stripped.
//     Malformed lines without '=' are skipped (parser is forgiving).
//     On failure (e.g. file can't be opened) returns false and sets err.
// - get(section, key, default_val):
//     Returns the configured value or default_val if not found.
//
// The parser stores data in an unordered_map<string, unordered_map<string,string>>
// where outer map keys are section names and inner map keys are keys within the section.

#include "config.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

// Trim whitespace from both ends of a string.
// Uses unsigned char cast for correct behaviour with negative char values.
static inline std::string trim(const std::string &s) {
    size_t b = 0;
    size_t e = s.size();
    // advance b until first non-space
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    // move e back until last non-space
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

// Load INI-style config file.
// - path: path to INI file
// - err: output error message on failure
// Returns true on success, false on failure.
bool Config::loadFromFile(const std::string &path, std::string &err) {
    data_.clear();

    // open file stream
    std::ifstream ifs(path);
    if (!ifs) {
        // provide useful error message to caller
        err = "Could not open config file: " + path;
        return false;
    }

    std::string line;
    std::string current_section; // holds current [section] name; empty string for top-level (no section)
    size_t lineno = 0;
    // Read file line by line to preserve simple parser semantics and better error reporting if needed.
    while (std::getline(ifs, line)) {
        ++lineno;
        std::string s = trim(line);
        if (s.empty()) continue;                // skip blank lines
        if (s[0] == ';' || s[0] == '#') continue; // skip full-line comments

        // Section header: [section-name]
        if (s.front() == '[' && s.back() == ']') {
            current_section = trim(s.substr(1, s.size() - 2));
            continue;
        }

        // Expect key=value pairs. If no '=' present, ignore the line (for robustness).
        auto eq = s.find('=');
        if (eq == std::string::npos) {
            // malformed/unknown line - ignore but continue parsing the rest of the file
            continue;
        }

        // Extract key and value, trimming both sides.
        std::string key = trim(s.substr(0, eq));
        std::string val = trim(s.substr(eq + 1));

        // Remove inline comments from the value (e.g., "value ; comment" or "value # comment").
        // Find earliest occurrence of either ';' or '#' in the value and trim there.
        size_t cpos = std::string::npos;
        for (char c : {';', '#'}) {
            auto p = val.find(c);
            if (p != std::string::npos) {
                if (cpos == std::string::npos) cpos = p;
                else cpos = std::min(cpos, p);
            }
        }
        if (cpos != std::string::npos) val = trim(val.substr(0, cpos));

        // Store the key/value under the current section. Empty section name means top-level.
        data_[current_section][key] = val;
    }
    return true;
}

// Retrieve a value from the parsed config.
// If section or key does not exist, return default_val.
std::string Config::get(const std::string &section, const std::string &key, const std::string &default_val) const {
    auto sit = data_.find(section);
    if (sit == data_.end()) return default_val;
    auto kit = sit->second.find(key);
    if (kit == sit->second.end()) return default_val;
    return kit->second;
}