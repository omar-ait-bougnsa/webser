#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>


class Tools
{
private:
public:
    Tools();
    ~Tools();

    static std::string                      readFile(const std::string &filePath);
    static std::vector<std::string>         split(std::string line, char target);
    static std::vector<std::string>         split_withspace(std::string str);
    static std::string                      int_to_string(int num);;

};


#endif