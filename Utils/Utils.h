#pragma once
#include <vector>
#include <string>

std::vector<uint8_t> ReadFile(const std::wstring& filename);
void WriteFile(const std::wstring& filename, const std::vector<uint8_t>& data);

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
