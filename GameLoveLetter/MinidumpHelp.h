// http://wiredlayer.tistory.com/20에 나온 소스를 간단하게 클래스로 변경해놓기만 한 파일입니다^^
#pragma once

#include <Windows.h>
#include <string>

LONG WINAPI my_top_level_filter(__in PEXCEPTION_POINTERS pExceptionPointer);
class MinidumpHelp
{
private:
	std::string format_arg_list(const char *fmt, va_list args);
	std::string format_string(const char *fmt, ...);
	std::wstring s2ws(const std::string& s);
	
public:
	std::wstring get_dump_filename();
	MinidumpHelp(void);
	~MinidumpHelp(void);
	
	void install_self_mini_dump();

};
