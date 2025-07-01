#pragma once
#include <iostream>
#include <string>
#include <sstream>
class Process {
public:
    std::string output{};
    int returnCode = 0;
    Process();
    void Exec(const std::string &cmd);
    
};
