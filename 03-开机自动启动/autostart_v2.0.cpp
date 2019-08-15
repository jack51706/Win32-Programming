#define _WIN32_WINNT 0x0400
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#include <windows.h>
#include <iostream>
#include <stdlib.h>
using namespace std;

void SetAutoRun()
{
	HKEY hKey;
	char szModule[_MAX_PATH];
	/*
	 * 32位目录:"HKEY_LOCAL_MACHINE\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
	 * 64位目录:"HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run"
	 * 网上推荐的做法RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE | KEY_WOW64_64KEY, &hKey);
	 */
	const wchar_t* strRegPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";	// 找到系统的启动项 
	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, strRegPath, 0, KEY_ALL_ACCESS, &hKey)== ERROR_SUCCESS) // 打开启动项       
	{
		GetModuleFileNameA(NULL, szModule, _MAX_PATH);	//获取自身路径
		RegSetValueExA(hKey, "test", 0, REG_SZ, (const BYTE*)szModule, strlen(szModule)); // 修改注册表
		RegCloseKey(hKey);	// 关闭注册表  
	}/*
	else
	{
		MessageBoxA(0, "系统参数错误,不能随系统启动", "Notice!", MB_OK);
	}*/
}

int main() {
	SetAutoRun();
	return 0;
};