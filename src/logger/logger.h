#pragma once
#include <iostream>
#include <string>
#include <map>

class Logger
{
public:
    std::map<int, bool> logLevels = {
        {1, true},  // INFO
        {2, true},  // WARN
        {3, true},  // ERROR
        {4, true}   // TOGGLE ALL LOGS
    };

    void setLogSwitch(const bool& value, const int& index){
        logLevels.at(index) = value;
    }

    void info(const std::string& message){
        if(logLevels.at(4) == false) return;
        if(logLevels.at(1) == true) std::cout << "[INFO] " << message << std::endl;
    }

    void error(const std::string& message){
        if(logLevels.at(4) == false) return;
        if(logLevels.at(2) == true) std::cerr << "[ERROR] " << message << std::endl;
    }

    void warn(const std::string& message){
        if(logLevels.at(4) == true) return;
        if(logLevels.at(3) == true) std::cout << "[WARN] " << message << std::endl;
    }
};
