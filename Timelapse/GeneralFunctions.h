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

//Convert AsciiToHex Cli version
static PUCHAR atohx(PUCHAR szDestination, LPCSTR szSource) {
	const PUCHAR szReturn = szDestination;
	for (int lsb, msb; *szSource; szSource += 2) {
		msb = tolower(*szSource);
		lsb = tolower(*(szSource + 1));
		msb -= isdigit(msb) ? 0x30 : 0x57;
		lsb -= isdigit(lsb) ? 0x30 : 0x57;
		if ((msb < 0x0 || msb > 0xf) || (lsb < 0x0 || lsb > 0xf)) {
			*szReturn = 0;
			return nullptr;
		}
		*szDestination++ = static_cast<char>(lsb | (msb << 4));
	}
	*szDestination = 0;
	return szReturn;
}

//Convert AsciiToHex C++ version
static char* AsciiToHex(char *szDestination, const char *szSource) {
	char *szReturn = szDestination;
	for (int lsb, msb; *szSource; szSource += 2) {
		msb = tolower(*szSource);
		lsb = tolower(*(szSource + 1));
		msb -= isdigit(msb) ? 0x30 : 0x57;
		lsb -= isdigit(lsb) ? 0x30 : 0x57;
		if ((msb < 0x0 || msb > 0xf) || (lsb < 0x0 || lsb > 0xf)) {
			*szReturn = 0;
			return nullptr;
		}
		*szDestination++ = static_cast<char>(lsb | (msb << 4));
	}
	*szDestination = 0;
	return szReturn;
}