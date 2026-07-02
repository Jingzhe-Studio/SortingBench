#include "DataReader.h"

#include <cstdint>
#include <fstream>
#include <iostream>

namespace {
    constexpr uint32_t kMagic = 0x54414453;  // "SDAT" LE
}

std::vector<int> DataReader::readBinary(
    const std::string& filePath)
{
    std::ifstream in(filePath, std::ios::binary);
    if (!in) return {};

    uint32_t magic = 0;
    uint64_t count = 0;

    in.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    in.read(reinterpret_cast<char*>(&count), sizeof(count));

    if (!in || magic != kMagic || count == 0) {
        return {};
    }

    std::vector<int> data(static_cast<size_t>(count));
    in.read(reinterpret_cast<char*>(data.data()),
            static_cast<std::streamsize>(count * sizeof(int)));

    if (!in) return {};
    return data;
}

std::vector<int> DataReader::readText(
    const std::string& filePath)
{
    std::ifstream in(filePath);
    if (!in) return {};

    std::vector<int> data;
    int value = 0;
    while (in >> value) {
        data.push_back(value);
    }
    return data;
}
