#pragma once
#include <excpt.h>
#include <memoryapi.h>
#include <winbase.h>
#include <msclr/marshal_cppstd.h>

#define NewThread(threadFunc) CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&threadFunc, NULL, NULL, NULL);

#pragma region General Functions
static void MakePageWritable(ULONG ulAddress, ULONG ulSize) {
	MEMORY_BASIC_INFORMATION* mbi = new MEMORY_BASIC_INFORMATION;
	VirtualQuery((PVOID)ulAddress, mbi, ulSize);
	if (mbi->Protect != PAGE_EXECUTE_READWRITE) {
		ULONG* ulProtect = new unsigned long;
		VirtualProtect((PVOID)ulAddress, ulSize, PAGE_EXECUTE_READWRITE, ulProtect);
		delete ulProtect;
	}
	delete mbi;
}

static std::vector<char> HexToBytes(const std::string& hex) {
	std::vector<char> bytes;

	for (unsigned int i = 0; i < hex.length(); i += 2) {
		std::string byteString = hex.substr(i, 2);
		char byte = (char)strtol(byteString.c_str(), NULL, 16);
		bytes.push_back(byte);
	}

	return bytes;
}

int char2int(char input)
{
	if (input >= '0' && input <= '9')
		return input - '0';
	if (input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if (input >= 'a' && input <= 'f')
		return input - 'a' + 10;
	throw std::invalid_argument("Invalid input string");
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
void hex2bin(const char* src, char* target)
{
	while (*src && src[1])
	{
		*(target++) = char2int(*src) * 16 + char2int(src[1]);
		src += 2;
	}
}



static void Jump(ULONG ulAddress, PVOID Function, unsigned Nops) {
	MakePageWritable(ulAddress, Nops + 5);
	*(UCHAR*)ulAddress = 0xE9;
	*(ULONG*)(ulAddress + 1) = (int)(((int)Function - (int)ulAddress) - 5);
	memset((PVOID)(ulAddress + 5), 0x90, Nops);
}

static ULONG ReadPointer(ULONG ulBase, int iOffset) {
	__try { return *(ULONG*)(*(ULONG*)ulBase + iOffset); }
	__except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static LONG ReadPointerSigned(ULONG ulBase, int iOffset) {
	__try { return *(LONG*)(*(ULONG*)ulBase + iOffset); }
	__except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static int ReadPointerSignedInt(ULONG ulBase, int iOffset) {
	__try { return *(LONG*)(*(ULONG*)ulBase + iOffset); }
	__except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static double ReadPointerDouble(ULONG ulBase, int iOffset) {
	__try { return *(double*)(*(ULONG*)ulBase + iOffset); }
	__except (EXCEPTION_EXECUTE_HANDLER) { return NULL; }
}

static LPCSTR ReadPointerString(ULONG ulBase, int iOffset) {
	__try { return (LPCSTR)(*(ULONG*)ulBase + iOffset); }
	__except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
}

static UINT8 readCharValueZtlSecureFuse(int at) {
	UINT8 result;
	try {
		UCHAR v2 = *(PUCHAR)at;
		at = *(PUINT8)(at + 1);
		result = at ^ v2;
	}
	catch (...) { return 0; }
	return result;
}

static INT16 readShortValueZtlSecureFuse(int a1) {
	PUINT8 v2 = (PUINT8)(a1 + 2);
	DWORD v4 = (DWORD)&a1 - a1;
	try {
		v2[v4] = *v2 ^ *(v2 - 2);
		v2++;
		v2[v4] = *v2 ^ *(v2 - 2);
	}
	catch (...) { return 0; }
	return HIWORD(a1);
}

inline unsigned int readLongValueZtlSecureFuse(ULONG *a1) {
	try {
		return *a1 ^ _rotl(a1[1], 5);
	}
	catch (...) { return 0; }
}

#pragma unmanaged
static LONG_PTR ReadMultiPointerSigned(LONG_PTR ulBase, int level, ...) {
	LONG_PTR ulResult = 0;
	va_list vaarg;
	va_start(vaarg, level);

	if (!IsBadReadPtr((PVOID)ulBase, sizeof(LONG_PTR))) {
		ulBase = *(LONG_PTR*)ulBase;
		for (int i = 0; i < level; i++) {
			const int offset = va_arg(vaarg, int);
			if (IsBadReadPtr((PVOID)(ulBase + offset), sizeof(LONG_PTR))) {
				va_end(vaarg);
				return 0;
			}
			ulBase = *(LONG_PTR*)(ulBase + offset);
		}
		ulResult = ulBase;
	}

	va_end(vaarg);
	return ulResult;
}

static PCHAR ReadMultiPointerString(LONG_PTR ulBase, int level, ...) {
	PCHAR ulResult = nullptr;
	va_list vaarg;
	va_start(vaarg, level);

	if (!IsBadReadPtr((PVOID)ulBase, sizeof(LONG_PTR))) {
		ulBase = *(LONG_PTR*)ulBase;
		for (int i = 0; i < level; i++) {
			const int offset = va_arg(vaarg, int);
			if (IsBadReadPtr((PVOID)(ulBase + offset), sizeof(LONG_PTR))) {
				va_end(vaarg);
				return nullptr;
			}
			ulBase = *(LONG_PTR*)(ulBase + offset);
		}
		ulResult = (PCHAR)ulBase;
	}

	va_end(vaarg);
	return ulResult;
}

static void WriteMemory(ULONG ulAddress, ULONG ulAmount, ...) {
	va_list va;
	va_start(va, ulAmount);

	MakePageWritable(ulAddress, ulAmount);
	for (ULONG ulIndex = 0; ulIndex < ulAmount; ulIndex++)
		*(UCHAR*)(ulAddress + ulIndex) = va_arg(va, UCHAR);

	va_end(va);
}

#pragma managed
static bool WritePointer(ULONG ulBase, int iOffset, int iValue) {
	__try { *(int*)(*(ULONG*)ulBase + iOffset) = iValue; return true; }
	__except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
#pragma endregion