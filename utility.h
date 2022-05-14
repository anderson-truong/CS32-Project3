#ifndef UTILITY_H
#define UTILITY_H

#include <string>

// Stores ship type data
struct ShipType
{
    int length;
    char symbol;
    std::string name;
    ShipType(int length, char symbol, std::string name) : length(length), symbol(symbol), name(name) {}
};

#endif