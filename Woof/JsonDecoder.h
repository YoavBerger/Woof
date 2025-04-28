#pragma once
#include <fstream>
#include <map>
#include <string>
#include <stdexcept>
#include <vector>
#include "json.hpp"
#include "SecurityChecker.h"


using json = nlohmann::json;



class JsonDecoder
{
public:
    JsonDecoder(const JsonDecoder&) = delete;
    JsonDecoder& operator=(const JsonDecoder&) = delete;

    static std::shared_ptr<JsonDecoder> getInstance();

    std::map<std::string, std::string> getSettings();
    std::map<std::string, std::vector<Error>> getSubDirectoriesSettings();
    static std::unordered_map<std::string, std::vector<std::string>> jsonArraysToMapOfVectors(const std::string& path);

    JsonDecoder();
private:
    void loadSettings();
    void loadSubDirectoriesSettings();
    static std::vector<Error> parseNumbersFromBrackets(const std::string& bracketsString);
    std::map<std::string, std::string> settings;
    std::map<std::string, std::vector<Error>> subDirectoriesSettings;
};