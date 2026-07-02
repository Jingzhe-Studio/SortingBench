#pragma once

#include <string>
#include <vector>

class DataReader {
public:
    /// Read a binary file written by DataWriter::writeBinary.
    /// Returns an empty vector on failure.
    static std::vector<int> readBinary(
        const std::string& filePath);

    /// Read a plain-text file (one integer per line).
    /// Returns an empty vector on failure.
    static std::vector<int> readText(
        const std::string& filePath);
};
