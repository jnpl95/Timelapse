#pragma once
#include <string>
#include <msclr/marshal_cppstd.h>

//Convert String^ to std::string
static std::string ConvertSystemToStdStr(String^ str1) {
	return msclr::interop::marshal_as<std::string>(str1);
}

//Convert std::string to String^
static String^ ConvertStdToSystemStr(std::string str1) {
	return gcnew String(str1.c_str());
}

//TODO: If these funcs are not needed, delete later
String^ CharToHex(char c) {
	char a[100];
	sprintf_s(a, "%x", c);
	return gcnew String(a);
}

String^ IntToHex(int c) {
	char a[100];
	sprintf_s(a, "%x", c);
	return gcnew String(a);
}
