#include <Windows.h>
#include <Console.h>
#include <vector>

#include "Utils.h"

constexpr const wchar_t* kInputFile = L"input.txt";
constexpr const wchar_t* kKeyFile = L"key.txt";
constexpr const wchar_t* kEncryptedFile = L"encrypted.txt";
constexpr const wchar_t* kDecryptedFile = L"decrypted.txt";

const uint32_t kC = 0x2AAAAAAAu;

uint32_t GammmaMul(uint32_t gamma, uint32_t x) {
    if (x == UINT32_MAX) {
        return UINT32_MAX;
    }

    return (static_cast<uint64_t>(gamma) * static_cast<uint64_t>(x)) % UINT32_MAX;
}

uint32_t ModInverse(uint32_t a, uint32_t b) {
    // ModInverse using Extended Euclidean Algorithm
    if (b == 1) {
        return 1;
    }

    int b0 = b;
    int x0 = 0;
    int x1 = 1;
    int t;
    int q;

    while (a > 1) {
        q = a / b;

        t = b;
        b = a % b;
        a = t;

        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }

    if (x1 < 0) {
        x1 += b0;
    }

    return x1;
}


uint32_t InverseGammaMul(uint32_t gamma, uint32_t result) {
    if (result == UINT32_MAX) {
        return UINT32_MAX;
    }

    return (static_cast<uint64_t>(result) * static_cast<uint64_t>(ModInverse(gamma, UINT32_MAX))) % UINT32_MAX;
}

void F(uint32_t& x0, uint32_t& x1, uint32_t& x2, uint32_t& x3) {
    // Stage 1
    static const uint32_t kC0 = 0x025F1CDBu;
    static const uint32_t kC1 = GammmaMul(kC0, 2);
    static const uint32_t kC2 = GammmaMul(kC0, 8);
    static const uint32_t kC3 = GammmaMul(kC0, 128);
    x0 = GammmaMul(kC0, x0);
    x1 = GammmaMul(kC1, x1);
    x2 = GammmaMul(kC2, x2);
    x3 = GammmaMul(kC3, x3);

    // Stage 2
    if (x0 & 1) {
        x0 ^= kC;
    }
    else if (!(x3 & 1)) {
        x3 ^= kC;
    }

    // Stage 3
    uint32_t _x0 = x0;
    uint32_t _x1 = x1;
    uint32_t _x2 = x2;
    uint32_t _x3 = x3;
    x0 = _x3 ^ _x0 ^ _x1;
    x1 = _x0 ^ _x1 ^ _x2;
    x2 = _x1 ^ _x2 ^ _x3;
    x3 = _x2 ^ _x3 ^ _x0;
}

void FInverse(uint32_t& x0, uint32_t& x1, uint32_t& x2, uint32_t& x3) {
    // Stage 3
    uint32_t _x0 = x0;
    uint32_t _x1 = x1;
    uint32_t _x2 = x2;
    uint32_t _x3 = x3;
    x0 = _x3 ^ _x0 ^ _x1;
    x1 = _x0 ^ _x1 ^ _x2;
    x2 = _x1 ^ _x2 ^ _x3;
    x3 = _x2 ^ _x3 ^ _x0;

    // Stage 2
    if (x0 & 1) {
        x0 ^= kC;
    }
    else if (!(x3 & 1)) {
        x3 ^= kC;
    }

    // Stage 1
    static const uint32_t kC0 = 0x025F1CDBu;
    static const uint32_t kC1 = GammmaMul(kC0, 2);
    static const uint32_t kC2 = GammmaMul(kC0, 8);
    static const uint32_t kC3 = GammmaMul(kC0, 128);
    x0 = InverseGammaMul(kC0, x0);
    x1 = InverseGammaMul(kC1, x1);
    x2 = InverseGammaMul(kC2, x2);
    x3 = InverseGammaMul(kC3, x3);
}

std::vector<uint32_t> Encrypt(const std::vector<uint32_t>& data, const std::vector<uint32_t>& key) {
    std::vector<uint32_t> result(data);
    // 128-bit padding
    result.resize(data.size() + (4 - data.size() % 4) % 4, 0);

    for (size_t i = 0; i < result.size(); i += 4) { // For each block
        for (size_t r = 0; r < 6; ++r) { // For each round
            for (size_t k = 0; k < 4; ++k) { // For each subblock
                result[i + (k + r) % 4] ^= key[(k + r) % 4];
            }

            F(result[i], result[i + 1], result[i + 2], result[i + 3]);
        }
    }

    return result;
}

std::vector<uint32_t> Decrypt(const std::vector<uint32_t>& data, const std::vector<uint32_t>& key) {
    std::vector<uint32_t> result(data);
    // 128-bit padding
    result.resize(data.size() + (4 - data.size() % 4) % 4, 0);

    for (size_t i = 0; i < result.size(); i += 4) { // For each block
        for (size_t r = 0; r < 6; ++r) { // For each round
            FInverse(result[i], result[i + 1], result[i + 2], result[i + 3]);

            for (size_t k = 0; k < 4; ++k) { // For each subblock
                result[i + (k + r) % 4] ^= key[(k + r) % 4];
            }
        }
    }

    return result;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    std::vector<uint32_t> input = VecConvert<uint32_t, uint8_t>(ReadFile(kInputFile));
    std::vector<uint32_t> key = VecConvert<uint32_t, uint8_t>(ReadFile(kKeyFile));

    // Encryption
    std::vector<uint32_t> encrypted = Encrypt(input, key);
    std::vector<uint8_t> encryptedB = VecConvert<uint8_t, uint32_t>(encrypted);
    WriteFile(kEncryptedFile, encryptedB);

    // Decryption
    std::vector<uint32_t> decrypted = Decrypt(encrypted, key);
    std::vector<uint8_t> decryptedB = VecConvert<uint8_t, uint32_t>(decrypted);
    WriteFile(kDecryptedFile, decryptedB);

    return 0;
}
