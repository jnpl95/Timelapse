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