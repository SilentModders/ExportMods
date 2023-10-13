// Main file for export

#include "pch.h"
#include "operations.h"

// Will become INI options
constexpr bool LOGGING_ENABLED = true;
constexpr auto FILE_NAME = "Modlist.htm";

// This will be where vortex.deployment.json resides.
char Game_Path[MAX_PATH];
std::ofstream output;

bool GetPath()
{
	return GetCurrentDirectoryA(MAX_PATH, Game_Path);
}

void LogLine(std::string line)
{
	if (!LOGGING_ENABLED)
		return;

	// Start the line with a timestamp.
	time_t now = time(0);
	char sTime[26];
	ctime_s(sTime, 26, &now);
	// Remove extra line break.
	sTime[strlen(sTime) - 1] = '\0';

	std::string stamp("[");
	stamp.append(sTime);
	stamp.append("] ");

	output << stamp << line << std::endl;
}

bool StartUp()
{
	// Start with the current folder as our path.
	bool bPath = GetPath();

	if (LOGGING_ENABLED)
	{
		// Open the log file
		output.open("ExportMods.log");
		// Stop if the log can't open.
		if (!output)
			return false;
	}

	// Stop in case there's no path.
	if (!bPath)
	{
		LogLine("Unable to determine path!");
		output.close();
		return true;
	}

	// Log the current folder
	std::string line("Started from ");
	line.append(Game_Path);
	LogLine(line);

	output.close();
	return true;
}
