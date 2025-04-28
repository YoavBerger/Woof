#include "JsonDecoder.h"
#include <sstream>
#include <iostream>

std::shared_ptr<JsonDecoder> JsonDecoder::getInstance()
{
    static std::shared_ptr<JsonDecoder> instance = std::make_shared<JsonDecoder>();
    return instance;
}

JsonDecoder::JsonDecoder()
{

}

std::unordered_map<std::string, std::vector<std::string>> JsonDecoder::jsonArraysToMapOfVectors(const std::string& path)
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
                    }
                    else {
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

std::map<std::string, std::string> JsonDecoder::getSettings()
{
    if(settings.empty())
        loadSettings();
    return settings;
}

std::map<std::string, std::vector<Error>> JsonDecoder::getSubDirectoriesSettings() 
{
    if(subDirectoriesSettings.empty())
        loadSubDirectoriesSettings();
    return subDirectoriesSettings;
}

std::vector<Error> JsonDecoder::parseNumbersFromBrackets(const std::string& bracketsString) {
    std::vector<Error> result;
    // Remove leading '(' and trailing ')'
    std::string trimmedStr = bracketsString.substr(1, bracketsString.length() - 2);
    // Split by '.'
    std::istringstream ss(trimmedStr);
    std::string number;
    while (std::getline(ss, number, '.')) {
        int value = std::stoi(number);
        // Convert int to Error enum, checking if it's in valid range
        if (value >= (int) Error::DDOS && value <= (int) Error::CSRF) {
            result.push_back(static_cast<Error>(value));
        }
        else {
            throw std::runtime_error("Invalid error value: " + number);
        }
    }
    return result;
}

void JsonDecoder::loadSettings() {
    try {
        std::ifstream file("Settings.json");
        if (!file.is_open()) {
            throw std::runtime_error("Could not open the JSON file Settings.json");
        }
        nlohmann::json jsonData;
        file >> jsonData;
        settings.clear();

        // Changed this part to read direct key-value pairs since there's no "settings" wrapper
        for (const auto& element : jsonData.items()) {
            settings[element.key()] = element.value().dump();
        }

        file.close();
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error loading settings: " + std::string(e.what()));
    }
}

void JsonDecoder::loadSubDirectoriesSettings() {
    try {
        std::ifstream file("SubSettings.json");
        if (!file.is_open()) {
            throw std::runtime_error("Could not open the JSON file: SubSettings.json");
        }
        nlohmann::json jsonData;
        file >> jsonData;
        subDirectoriesSettings.clear();

        // Changed this part to read direct key-value pairs since there's no "subDirectories" wrapper
        for (const auto& element : jsonData.items()) {
            subDirectoriesSettings[element.key()] = parseNumbersFromBrackets(element.value().get<std::string>());
        }

        file.close();
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error loading subdirectory settings: " + std::string(e.what()));
    }
}