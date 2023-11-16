#define NOMINMAX
#include <Windows.h>
#include <Console.h>
#include <fstream>
#include <string>
#include <vector>
#include <Utils.h>

constexpr const wchar_t* kInputFile = L"input.txt";
constexpr const wchar_t* kEncodedFile = L"encoded.txt";
constexpr const wchar_t* kDecodedFile = L"decoded.txt";

typedef int lzss_size;

const lzss_size WINDOW_SIZE = 4096;
const lzss_size MIN_MATCH_SIZE = 1;
const uint8_t LITERAL_MARKER = 0;
const uint8_t MATCH_MARKER = 1;

inline void VecEmplaceValue(std::vector<uint8_t>& vec, lzss_size value) {
    for (size_t i = 0; i < sizeof(lzss_size); ++i) {
        vec.emplace_back((uint8_t)((value >> (8 * i)) & 0xFF));
    }
}

inline int VecGetValue(const std::vector<uint8_t>& vec, size_t index) {
    lzss_size result = 0;
    for (size_t i = 0; i < sizeof(lzss_size); ++i) {
        result |= (lzss_size)(vec[index + i]) << (8 * i);
    }

    return result;
}

std::vector<uint8_t> LzssEncode(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> encoded;
    lzss_size index = 0;

    while (index < input.size()) {
        lzss_size matchLength = 0;
        lzss_size matchIndex = -1;

        for (lzss_size i = std::max(0, index - WINDOW_SIZE); i < index; ++i) {
            lzss_size j = 0;

            while (index + j < input.size() && input[i + j] == input[index + j]) {
                ++j;

                if (j >= MIN_MATCH_SIZE && j > matchLength) {
                    matchLength = j;
                    matchIndex = i;
                }
            }
        }

        if (matchLength >= MIN_MATCH_SIZE) {
            encoded.emplace_back(MATCH_MARKER);
            VecEmplaceValue(encoded, matchIndex);
            VecEmplaceValue(encoded, matchLength);
            index += matchLength;
        }
        else {
            encoded.emplace_back(LITERAL_MARKER);
            encoded.emplace_back(input[index++]);
        }
    }

    return encoded;
}

std::vector<uint8_t> LzssDecode(const std::vector<uint8_t>& encoded) {
    std::vector<uint8_t> decoded;
    lzss_size index = 0;

    while (index < encoded.size()) {
        uint8_t marker = encoded[index++];

        if (marker == LITERAL_MARKER) {
            decoded.emplace_back(encoded[index++]);
        }
        else if (marker == MATCH_MARKER) {
            lzss_size matchIndex = VecGetValue(encoded, index);
            index += sizeof(lzss_size);
            lzss_size matchLength = VecGetValue(encoded, index);
            index += sizeof(lzss_size);

            for (lzss_size i = 0; i < matchLength; ++i) {
                decoded.emplace_back(decoded[matchIndex + i]);
            }
        }
    }

    return decoded;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    std::vector<uint8_t> data = ReadFile<uint8_t>(kInputFile);
    std::vector<uint8_t> encoded = LzssEncode(data);
    WriteFile(kEncodedFile, encoded);
    std::vector<uint8_t> decoded = LzssDecode(encoded);
    WriteFile(kDecodedFile, decoded);
	return 0;
}
