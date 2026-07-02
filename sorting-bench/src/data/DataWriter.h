#pragma once

#include <cstddef>
#include <string>
#include <vector>

class DataWriter {
public:
    /// Write data as a compact binary file.  Returns true on success.
    /// Format: [magic:4B][count:8B][data:count*4B]
    static bool writeBinary(
        const std::vector<int>& data,
        const std::string& filePath);

    /// Write data as plain text, one integer per line.
    static bool writeText(
        const std::vector<int>& data,
        const std::string& filePath);
};
