#include<Windows.h>
#include<iostream>
#include<string>
#include<io.h>
#include<fcntl.h>
#include<fstream>

#pragma comment(lib, "Comctl32.lib")

wchar_t currentPath[MAX_PATH];//当前程序运行的路径
wchar_t apkPath[MAX_PATH];//apk所在的路径
wchar_t adbPath[MAX_PATH];//adb.exe所在的路径
wchar_t getApkNameCmd[MAX_PATH];//获取apk名称的命令
wchar_t adbInstallapkCmd[MAX_PATH];//adb.exe安装软件的命令

STARTUPINFO si;
PROCESS_INFORMATION pi;

int wmain()
{
	//设置控制台输出为UTF-16
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stdin), _O_U16TEXT);
	_setmode(_fileno(stderr), _O_U16TEXT);

	//初始化CreateProcess的参数
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));

	//获取当前运行路径
	GetModuleFileNameW(GetModuleHandle(NULL), currentPath, MAX_PATH);

	//截取路径到最后一个反斜杠
	for (LONGLONG i = wcslen(currentPath) - 1; i >= 0; i--)
	{
		if(currentPath[i] == L'\\')
		{
			currentPath[i + 1] = L'\0';
			break;
		}
	}

	//获取adb.exe路径
	wcscpy_s(adbPath, MAX_PATH, currentPath);
	wcscat_s(adbPath, MAX_PATH, L"adb\\adb.exe");
	std::wcout << L"adb路径如下：" << adbPath << L"\n";

	//启动adb服务
	wcscpy_s(adbInstallapkCmd, MAX_PATH, adbPath);
	wcscat_s(adbInstallapkCmd, MAX_PATH, L" start-server");
	std::wcout << adbInstallapkCmd << L"\n";
	CreateProcess(NULL, adbInstallapkCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	//获取apk安装包路径
	wcscpy_s(apkPath, MAX_PATH, currentPath);
	wcscat_s(apkPath, MAX_PATH, L"apks\\");
	std::wcout << L"扫描到的安装包如下：\n";

	//获取文件列表
	wcscpy_s(getApkNameCmd, MAX_PATH, L"cmd /c dir .\\apks /B > C:\\apkList.txt");
	CreateProcess(NULL, getApkNameCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	std::wifstream wifs;
	wifs.open("C:\\apkList.txt", std::ios::in);
	wifs >> std::noskipws;

	wchar_t buf[MAX_PATH * 512] = { 0 };
	wifs.read(buf, sizeof(buf));
	wifs.close();

	//分隔每个文件名
	wchar_t apkName[512][MAX_PATH] = { 0 };
	ULONGLONG apkCount = 0, wordCount = 0;

	for (ULONGLONG i = 0; i < wcslen(buf); i++)
	{
		if (buf[i] == L'\n')
		{
			apkCount++;
			wordCount = 0;
			continue;
		}
		apkName[apkCount][wordCount] = buf[i];
		wordCount++;
	}
	
	//输出扫描到的安装包
	for (ULONGLONG i = 0; i < apkCount; i++)
	{
		std::wcout << i + 1 << L": " << apkName[i] << L"\n";
	}

	//用户确认是否要安装
	std::wstring userInput;
	std::wcout << L"是否开始安装以上" << apkCount << L"个安装包？\n如果是，请完整地打出小写yes。\n>>";
	std::wcin >> userInput;
	if (wcscmp(userInput.c_str(), L"yes") != 0)
	{
		std::wcout << L"用户取消安装，程序结束。\n";
		ExitProcess(EXIT_FAILURE);
	}

	//开始安装apk
	std::wcout << L"开始安装...\n";
	for (ULONGLONG i = 0; i < apkCount; i++)
	{
		//初始化adb命令的头部
		wcscpy_s(adbInstallapkCmd, MAX_PATH, adbPath);
		wcscat_s(adbInstallapkCmd, MAX_PATH, L" install ");
		
		//连接apk路径和名称
		wcscat_s(adbInstallapkCmd, MAX_PATH, apkPath);
		wcscat_s(adbInstallapkCmd, MAX_PATH, apkName[i]);

		std::wcout << i + 1 << L": " << adbInstallapkCmd << L"\n";

		CreateProcess(NULL, adbInstallapkCmd, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	//结束adb服务
	wcscpy_s(adbInstallapkCmd, MAX_PATH, adbPath);
	wcscat_s(adbInstallapkCmd, MAX_PATH, L" kill-server");
	std::wcout << adbInstallapkCmd << L"\n";
	CreateProcess(NULL, adbInstallapkCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	MessageBox(GetConsoleWindow(), L"安装完成", L"提示", MB_OK | MB_SYSTEMMODAL | MB_TOPMOST | MB_SETFOREGROUND | MB_ICONINFORMATION);
	ExitProcess(EXIT_SUCCESS);
}