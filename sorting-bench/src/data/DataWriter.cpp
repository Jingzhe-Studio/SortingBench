#include "DataWriter.h"

#include <cstdint>
#include <fstream>

namespace {
    // Little-endian magic: 'S' 'D' 'A' 'T'  (Sorting DATa)
    constexpr uint32_t kMagic   = 0x54414453;  // "SDAT" in little-endian
    constexpr size_t   kHeaderSize = sizeof(uint32_t) + sizeof(uint64_t);
}

bool DataWriter::writeBinary(
    const std::vector<int>& data,
    const std::string& filePath)
{
    std::ofstream out(filePath, std::ios::binary);
    if (!out) return false;

    uint32_t magic = kMagic;
    uint64_t count = static_cast<uint64_t>(data.size());

    out.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    out.write(reinterpret_cast<const char*>(data.data()),
              data.size() * sizeof(int));

    return out.good();
}

bool DataWriter::writeText(
    const std::vector<int>& data,
    const std::string& filePath)
{
    std::ofstream out(filePath);
    if (!out) return false;

    for (int value : data) {
        out << value << '\n';
    }
    return out.good();
}
