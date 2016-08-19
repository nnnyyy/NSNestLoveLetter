#include "StdAfx.h"
#include "MinidumpHelp.h"
#include <time.h>
#include <DbgHelp.h>


#pragma comment(lib,"DbgHelp.Lib")

MinidumpHelp::MinidumpHelp(void)
{
}

MinidumpHelp::~MinidumpHelp(void)
{
}

std::string MinidumpHelp::format_arg_list(const char *fmt, va_list args)
{
	if (!fmt) return "";
	int   result = -1, length = 256;
	char *buffer = 0;
	while (result == -1)
	{
		if (buffer) delete [] buffer;
		buffer = new char [length + 1];
		memset(buffer, 0, length + 1);
		result = _vsnprintf_s(buffer, length, _TRUNCATE, fmt, args);
		length *= 2;
	}
	std::string s(buffer);
	delete [] buffer;
	return s;
}

std::string MinidumpHelp::format_string(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	std::string s = format_arg_list(fmt, args);
	va_end(args);

	return s;
}

std::wstring MinidumpHelp::s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	::MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

std::wstring MinidumpHelp::get_dump_filename()
{
	time_t rawtime;
	struct tm timeinfo;

	std::string date_string;
	std::wstring date_wstring;

	std::wstring module_path;
	std::wstring dump_filename;

	static WCHAR ModulePath[1024];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	date_string = format_string("%d.%02d.%02d.%02d.%02d.%02d", 
		timeinfo.tm_year + 1900,
		timeinfo.tm_mon + 1,
		timeinfo.tm_mday,
		timeinfo.tm_hour,
		timeinfo.tm_min,
		timeinfo.tm_sec);
	date_wstring = s2ws(date_string);

	if (::GetModuleFileNameW(0, ModulePath, sizeof(ModulePath) /sizeof(WCHAR)) == 0)
	{
		return std::wstring();
	}

	module_path = ModulePath;
	dump_filename.assign(module_path, 0, module_path.rfind(L"\\") + 1);

	dump_filename = dump_filename + date_wstring + L".dmp";

	return dump_filename;
}

LONG WINAPI my_top_level_filter(__in PEXCEPTION_POINTERS pExceptionPointer)
{
	MinidumpHelp mini;
	MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation;
	std::wstring dump_filename;

	MinidumpExceptionInformation.ThreadId = ::GetCurrentThreadId();
	MinidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
	MinidumpExceptionInformation.ClientPointers = FALSE;


	dump_filename = mini.get_dump_filename();
	if (dump_filename.empty() == true)
	{
		::TerminateProcess(::GetCurrentProcess(), 0);
	}

	HANDLE hDumpFile = ::CreateFileW(dump_filename.c_str(),
		GENERIC_WRITE, 
		FILE_SHARE_WRITE, 
		NULL, 
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	MiniDumpWriteDump(GetCurrentProcess(),
		GetCurrentProcessId(),
		hDumpFile,
		MiniDumpNormal,
		&MinidumpExceptionInformation,
		NULL,
		NULL);
	::TerminateProcess(::GetCurrentProcess(), 0);

	return 0;
}

void MinidumpHelp::install_self_mini_dump()
{
	SetUnhandledExceptionFilter(my_top_level_filter);
}
