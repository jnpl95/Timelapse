#include "Log.h"

Void Log::WriteLine(String^ Message)
{
	StreamWriter^ sw = {};

	try
	{
		sw = gcnew StreamWriter(Log::GetLogPath(), true);

		if (Message == String::Empty) 
			sw->WriteLine();
		else 
			sw->WriteLine(DateTime::Now.ToString() + ": " + Message);
	}

	catch (Exception^)
	{
		// Can't log here :(
	}

	finally
	{
		if (sw) 
			delete static_cast<IDisposable^>(sw);
	}
}

Void Log::WriteLine()
{
	WriteLine(String::Empty);
}

String^ Log::GetLogPath() {

	String^ AppDataFolder = Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData);
	String^ LogsFilePath = {};

	try
	{
		String^ TimelapseFolderPath = Path::Combine(AppDataFolder, "Timelapse");
		LogsFilePath = Path::Combine(TimelapseFolderPath, "log.txt");
	}

	catch (Exception^ ex)
	{
		Log::WriteLine("Exception occured while generating path to log file" + ex->Message);
	}

	Log::WriteLine("Generated path to Timelapse log file at " + ": " + LogsFilePath);

	return LogsFilePath;
}