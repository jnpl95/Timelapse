#pragma once
using namespace System;
using namespace IO;

public ref class Log sealed 
{
public:
	static Void WriteLine(String^ Message);
	static Void WriteLine();
	static String^ GetLogPath();
};