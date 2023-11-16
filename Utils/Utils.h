#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

template <typename T, typename = std::enable_if_t<std::conjunction_v<std::is_integral<T>>>>
std::vector<T> ReadFile(const std::wstring& filename) {
    std::ifstream input(filename, std::ios::binary);
    if (!input.is_open()) {
        throw std::runtime_error("Error opening file.");
    }

    input.seekg(0, std::ios::end);
    std::streamsize fileSize = input.tellg();
    input.seekg(0, std::ios::beg);

    size_t roundedSize = (fileSize + sizeof(T) - 1) / sizeof(T);
    std::vector<T> buffer(roundedSize);
    if (roundedSize != 0) {
        *(buffer.end() - 1) = 0;
        if (!input.read((char*)buffer.data(), fileSize)) {
            throw std::runtime_error("Error reading file.");
        }
    }

    return buffer;
}

template <typename T, typename = std::enable_if_t<std::conjunction_v<std::is_integral<T>>>>
void WriteFile(const std::wstring& filename, const std::vector<T>& data) {
    std::ofstream output(filename, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        throw std::runtime_error("Error opening file for writing.");
    }

    if (!output.write((const char*)data.data(), data.size() * sizeof(T))) {
        throw std::runtime_error("Error writing to file.");
    }
}

template <typename T, typename F,
    typename = std::enable_if_t<std::conjunction_v<std::is_integral<T>, std::is_unsigned<T>, std::is_integral<F>, std::is_unsigned<F>>>>
    std::vector<T> VecConvert(const F* data, size_t length) {
    std::vector<T> result((length * sizeof(F) + sizeof(T) - 1) / sizeof(T));
    if (length > 0) {
        result.back() = 0;
        std::memcpy(result.data(), data, length * sizeof(F));
    }

    return result;
}

template <typename T, typename F,
    typename = std::enable_if_t<std::conjunction_v<std::is_integral<T>, std::is_unsigned<T>, std::is_integral<F>, std::is_unsigned<F>>>>
    std::vector<T> VecConvert(const std::vector<F>& data) {
    return VecConvert<T, F>(data.data(), data.size());
}
