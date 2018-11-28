#pragma once
using namespace System;
using namespace IO;

public ref class Log sealed {
	public:
		static void WriteLine(String^ Message);
		static void WriteLine();
		static void WriteLineToConsole(String ^ str);
		static String^ GetLogPath();
};