#include <Windows.h>
#include <Console.h>
#include <Utils.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
	Console::GetInstance()->WPrintF(L"SUCK MY DICK!!\n");
	Console::GetInstance()->Pause();
	return 0;
}
