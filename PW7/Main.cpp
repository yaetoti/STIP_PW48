#include <Windows.h>
#include <Console.h>
#include <Utils.h>

#include "HashMD2.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    std::string message = "";
    std::vector<uint8_t> data(message.begin(), message.end());
    std::vector<uint8_t> hash = HashMD2(data);

    for (uint8_t byte : hash) {
        Console::GetInstance()->WPrintF(L"%02x", byte);
    }

	Console::GetInstance()->Pause();
	return 0;
}
