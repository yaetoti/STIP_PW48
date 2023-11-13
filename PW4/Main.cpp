#include <Windows.h>
#include <Console.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
	Console::GetInstance()->WPrintF(L"Hello, PW4\n");
	Console::GetInstance()->Pause();
	return 0;
}
