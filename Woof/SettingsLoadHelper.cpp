#include "SettingsLoadHelper.h"

std::unordered_map<std::string, std::string> SettingsLoadHelper::jsonToMap(const std::string& path)
{
    std::unordered_map<std::string, std::string> jsonMap;
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open the JSON file: " + path);
        }
        nlohmann::json jsonData;
        file >> jsonData;
        jsonMap.clear();

        for (const auto& element : jsonData.items()) {
            jsonMap[element.key()] = element.value().dump();
        }

        file.close();
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error loading jsonMap: " + std::string(e.what()));
    }
    return jsonMap;
}

std::unordered_map<std::string, std::vector<std::string>> SettingsLoadHelper::jsonArraysToMapOfVectors(const std::string& path)
{
    std::unordered_map<std::string, std::vector<std::string>> result;
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open the JSON file: " + path);
        }
        nlohmann::json jsonData;
        file >> jsonData;
        
        for (const auto& element : jsonData.items()) {
            const std::string& key = element.key();
            const auto& value = element.value();
            
            if (value.is_array()) {
                std::vector<std::string> stringArray;
                for (const auto& item : value) {
                    if (item.is_string()) {
                        stringArray.push_back(item.get<std::string>());
                    } else {
                        stringArray.push_back(item.dump());
                    }
                }
                result[key] = stringArray;
            }
        }
        
        file.close();
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error loading JSON arrays: " + std::string(e.what()));
    }
    return result;
}

std::unordered_map<std::string, std::vector<std::string>> SettingsLoadHelper::jsonToMapOfStrings(const std::string& path)
{
    std::unordered_map<std::string, std::string> jsonMap = jsonToMap(path);
    std::unordered_map<std::string, std::vector<std::string>> jsonMapOfStrings;
    for (const auto& entry : jsonMap) {
        jsonMapOfStrings[entry.first] = splitValueIntoStrings(entry.second);
    }
    return jsonMapOfStrings;
}

std::vector<Error> SettingsLoadHelper::splitValueIntoErrors(const std::string& bracketsString)
{
    std::vector<std::string> errorStrings = splitValueIntoStrings(bracketsString);
    std::vector<Error> result;
    for (const auto& number : errorStrings)
    {
        int value = std::stoi(number);
        // Convert int to Error enum, checking if it's in valid range
        if (value >= (int) Error::SQLI && value <= (int) Error::DDOS) {
            result.push_back(static_cast<Error>(value));
        }
        else {
            throw std::runtime_error("Invalid error value: " + number);
        }
    }
    return result;
}

std::vector<std::string> SettingsLoadHelper::splitValueIntoStrings(const std::string& value)
{
    std::vector<std::string> result;
    // Remove leading '(' and trailing ')'
    std::string trimmedStr = value.substr(2, value.length() - 2);
    // Split by '.'
    std::istringstream ss(trimmedStr);
    std::string strLine;
    while (std::getline(ss, strLine, '.')) {
        result.push_back(strLine);
    }
    return result;
}

std::vector<std::string> SettingsLoadHelper::textFileToStrings(const std::string& path)
{
    std::vector<std::string> result;
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open the text file: " + path);
    }
    std::string line;
    while (std::getline(file, line)) {
        result.push_back(line);
    }
    file.close();
    return result;

}