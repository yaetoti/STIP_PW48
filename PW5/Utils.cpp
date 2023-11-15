#include <fstream>
#include <stdexcept>

#include "Utils.h"

std::vector<uint8_t> ReadFile(const std::wstring& filename) {
    std::ifstream input(filename, std::ios::binary);
    if (!input.is_open()) {
        throw std::runtime_error("Error opening file.");
    }

    input.seekg(0, std::ios::end);
    std::streamsize fileSize = input.tellg();
    input.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(fileSize));
    if (!input.read((char*)buffer.data(), fileSize)) {
        throw std::runtime_error("Error reading file.");
    }

    return buffer;
}

void WriteFile(const std::wstring& filename, const std::vector<uint8_t>& data) {
    std::ofstream output(filename, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        throw std::runtime_error("Error opening file for writing.");
    }

    if (!output.write((const char*)data.data(), data.size())) {
        throw std::runtime_error("Error writing to file.");
    }
}