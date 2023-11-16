#include <Windows.h>
#include <Console.h>
#include <Utils.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
	Console::GetInstance()->WPrintF(L"8008 69420\n");
	Console::GetInstance()->Pause();
	return 0;
}
