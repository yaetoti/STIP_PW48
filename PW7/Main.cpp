#include <Windows.h>
#include <Console.h>
#include <Utils.h>

#include "HashMD2.h"

constexpr const wchar_t* kInputFile1 = L"input1.txt";
constexpr const wchar_t* kInputFile2 = L"input2.txt";

bool IsPrime(int n) {
    if (n == 0 || n == 1) {
        return false;
    }

    for (int i = 2; i <= n / 2; ++i) {
        if (n % i == 0) {
            return false;
        }
    }

    return true;
}

int64_t GCD(int64_t a, int64_t b) {
    while (a && b) {
        if (a > b) {
            a %= b;
        }
        else {
            b %= a;
        }
    }

    return a + b;
}

// Modular Exponentiation (x^y % p)
int64_t ModExp(int64_t x, int64_t y, int64_t p) {
    int64_t result = 1;
    x = x % p;

    while (y > 0) {
        if (y & 1) {
            result = (result * x) % p;
        }

        y = y >> 1;
        x = (x * x) % p;
    }

    return result;
}

// Modular Multiplicative Inverse using Euclid Algorithm
int64_t ModInv(int64_t a, int64_t m) {
    int64_t m0 = m;
    int64_t t;
    int64_t q;
    int64_t x0 = 0;
    int64_t x1 = 1;

    if (m == 1) {
        return 0;
    }

    while (a > 1) {
        q = a / m;
        t = m;
        m = a % m;
        a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }

    if (x1 < 0) {
        x1 += m0;
    }

    return x1;
}

// Find e such that 1 < e < phi and gcd(e, phi) = 1
int64_t FindE(int64_t phi) {
    int64_t e = 2;

    while (e < phi) {
        if (GCD(e, phi) == 1) {
            return e;
        }
        
        e++;
    }
}

// Function to generate keys
void GenKeys(int64_t p, int64_t q, int64_t& n, int64_t& e, int64_t& d) {
    n = p * q;
    int64_t phi = (p - 1) * (q - 1);
    // Choose e such that 1 < e < phi and gcd(e, phi) = 1
    e = FindE(phi);
    // Calculate d (private key) using modular inverse
    d = ModInv(e, phi);
}

// Function to encrypt a byte using public key (n, e)
int64_t EncryptSHA(int64_t data, int64_t exponent, int64_t privateKey) {
    return ModExp(data, privateKey, exponent);
}

// Function to decrypt a byte using private key (n, d)
int64_t DecryptSHA(int64_t data, int64_t exponent, int64_t publicKey) {
    return ModExp(data, publicKey, exponent);
}

std::vector<int64_t> SignRSA(const std::vector<uint8_t>& data, int64_t exponent, int64_t privateKey) {
    std::vector<int64_t> signature(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        signature[i] = EncryptSHA(data[i], exponent, privateKey);
    }

    return signature;
}

bool VerifyRSA(const std::vector<int64_t>& signature, const std::vector<uint8_t>& hash, int64_t exponent, int64_t publicKey) {
    std::vector<uint8_t> decrypted(hash.size());
    for (size_t i = 0; i < signature.size(); ++i) {
        decrypted[i] = (uint8_t)DecryptSHA(signature[i], exponent, publicKey);
    }

    for (size_t i = 0; i < hash.size(); ++i) {
        if (hash[i] != decrypted[i]) {
            return false;
        }
    }

    return true;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    std::vector<uint8_t> input1(ReadFile<uint8_t>(kInputFile1));
    std::vector<uint8_t> input2(ReadFile<uint8_t>(kInputFile2));
    std::vector<uint8_t> hash1 = HashMD2(input1);
    std::vector<uint8_t> hash2 = HashMD2(input2);

    Console::GetInstance()->WPrintF(L"Hash 1: ");
    for (uint8_t byte : hash1) {
        Console::GetInstance()->WPrintF(L"%02x", byte);
    }
    Console::GetInstance()->WPrintF(L"\n");

    Console::GetInstance()->WPrintF(L"Hash 2: ");
    for (uint8_t byte : hash2) {
        Console::GetInstance()->WPrintF(L"%02x", byte);
    }
    Console::GetInstance()->WPrintF(L"\n");

    // SHA Init
    int64_t exponent = 0;
    int64_t privateKey = 0;
    int64_t publicKey = 0;
    // "Randomly" choosed large prime numbers p and q
    GenKeys(2393, 2713, exponent, privateKey, publicKey);
    Console::GetInstance()->WPrintF(L"RSA Private Key: %08x\n", privateKey);
    Console::GetInstance()->WPrintF(L"RSA Public Key: %08x\n", publicKey);
    Console::GetInstance()->WPrintF(L"RSA Exponent: %08x\n", exponent);

    // Signing
    std::vector<int64_t> signature(SignRSA(hash1, exponent, privateKey));
    Console::GetInstance()->WPrintF(L"Signature: ");
    for (int64_t byte : signature) {
        Console::GetInstance()->WPrintF(L"%08x", byte);
    }
    Console::GetInstance()->WPrintF(L"\n\n");

    // Verification (Original)
    Console::GetInstance()->WPrintF(L"Verification 1: ");
    if (VerifyRSA(signature, hash1, exponent, publicKey)) {
        Console::GetInstance()->WPrintF(L"SUCCEDED\n");
    }
    else {
        Console::GetInstance()->WPrintF(L"FAILED\n");
    }

    // Verification (Modified)
    Console::GetInstance()->WPrintF(L"Verification 2: ");
    if (VerifyRSA(signature, hash2, exponent, publicKey)) {
        Console::GetInstance()->WPrintF(L"SUCCEDED\n");
    }
    else {
        Console::GetInstance()->WPrintF(L"FAILED\n");
    }

	Console::GetInstance()->Pause();
	return 0;
}
