#pragma once

#include <unordered_map>
#include <unordered_set>

#include <string>
#include <fstream>
#include <stdexcept>
#include <vector>
#include "SecurityChecker.h"
#include "json.hpp"
#include <regex>

#include "HandleUrl.h"

using json = nlohmann::json;


class SettingsLoadHelper
{
public:
    //give either a string or a securityChecker for the corresponding url
    template <class T>
    static T getBestMatchUrl(const std::string& url, const std::unordered_map<std::string, T>& settings)
    {
        std::string decodedUrl = HandleUrl::urlDecode(url);
        // First try exact match
        auto it = settings.find(decodedUrl);
        if (it != settings.end()) {
            return it->second;
        }

        // Try matching with wildcards
        //std::string bestMatch = "";
        //size_t bestMatchLength = 0;
        //for (const auto& entry : settings) {
        //    const std::string& pattern = entry.first;
        //    // Skip if pattern is longer than url (impossible match)
        //    if (pattern.length() > url.length()) {
        //        continue;
        //    }

        //    bool isMatch = false;
        //    if (pattern.back() == '*') {  // Ends with wildcard
        //        std::string prefix = pattern.substr(0, pattern.length() - 1);
        //        if (url.substr(0, prefix.length()) == prefix) {
        //            isMatch = true;
        //        }
        //    }

        //    if (isMatch && pattern.length() > bestMatchLength) {
        //        bestMatch = pattern;
        //        bestMatchLength = pattern.length();
        //    }
        //}

        std::string bestMatch;
        int maxLength = 0;

        for (const auto& item : settings) {
            if (!isValidRegex(item.first)) continue; // Skip invalid patterns

            std::regex regexPattern(item.first);
            if (std::regex_search(url, regexPattern) && item.first.length() > maxLength) {
                maxLength = item.first.length();
                bestMatch = item.first;
            }
        }
        
        //std::string bestMatch = findUrlWithRegex(decodedUrl, settings);

        if (!bestMatch.empty()) {
            return settings.at(bestMatch);
        }

        return T();
    }

protected:
    /// <summary>
    /// takes a json and turns it into a map
    /// </summary>
    /// <param name="path">path of json file</param>
    /// <returns>map of json</returns>
    static std::unordered_map<std::string, std::string> jsonToMap(const std::string& path);

    /// <summary>
    /// takes a json and turns it into a map
    /// but the values are split like this:
    /// {"key1":"(1.2.3.ok)","key2":"(5.6.another.something)"} =>
    /// a map of the key as the key
    /// but the values are split into a vector of strings
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    static std::unordered_map<std::string, std::vector<std::string>> jsonToMapOfStrings(const std::string& path);

    /// <summary>
    /// takes a json with array values and turns it into a map of vectors
    /// {"key1": ["value1", "value2"], "key2": ["value3"]} =>
    /// a map with keys mapping to vectors of strings
    /// </summary>
    /// <param name="path">path of json file</param>
    /// <returns>map of keys to vectors of strings</returns>
    static std::unordered_map<std::string, std::vector<std::string>> jsonArraysToMapOfVectors(const std::string& path);

    /// <summary>
    /// gets a string like so
    /// "(1.5.6.7)" and splits it into a vector of errors
    /// 1-sqli
    /// 2-xss...
    /// like in the Enum errors
    /// </summary>
    static std::vector<Error> splitValueIntoErrors(const std::string& bracketsString);

    /// <summary>
    /// gets a string like so
    /// "(string1.string2.string3)" or "(1.5.6.7)" and splits it into a vector of strings
    /// </summary>
    static std::vector<std::string> splitValueIntoStrings(const std::string& value);

    /// <summary>
    /// gets a text file and turns it into a vector of strings of the lines
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    static std::vector<std::string> textFileToStrings(const std::string& path);
    
    static inline bool isValidRegex(const std::string& pattern)
    {
        return std::regex(pattern).mark_count() >= 0;
    }

    template <class T>
    static std::string findUrlWithRegex(const std::string& url, const std::unordered_map<std::string, T>& settings)
    {
        std::string bestMatch;
        int maxLength = 0;

        for (const auto& item : settings) {
            if (!isValidRegex(item.first)) continue; // Skip invalid patterns

            std::regex regexPattern(item.first);
            if (std::regex_search(url, regexPattern) && item.first.length() > maxLength) {
                maxLength = item.first.length();
                bestMatch = item.first;
            }
        }
        return bestMatch;
    }
};

