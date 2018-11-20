#include "Log.h"

void Log::WriteLine(String^ Message) {
	StreamWriter^ sw = {};

	try {
		sw = gcnew StreamWriter(GetLogPath(), true);

		if (Message == String::Empty)
			sw->WriteLine();
		else
			sw->WriteLine(DateTime::Now.ToString() + " : " + Message);
	} 
	catch (Exception^) {}
	finally {
		if (sw)
			delete static_cast<IDisposable^>(sw);
	}
}

void Log::WriteLine() {
	WriteLine(String::Empty);
}

String^ Log::GetLogPath() {
	String^ AppDataFolder = Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData);
	String^ LogsFilePath = {};

	try {
		String^ TimelapseFolderPath = Path::Combine(AppDataFolder, "Timelapse");
		LogsFilePath = Path::Combine(TimelapseFolderPath, "Log.txt");
	} 
	catch (Exception^){}

	return LogsFilePath;
}