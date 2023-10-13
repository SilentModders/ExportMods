// Main file for export

#include <cstdlib>
#include "pch.h"
#include "operations.h"
#include "Shlobj.h"

constexpr auto LOG_PATH = L"\\My Games\\Starfield\\SFSE\\Logs\\ExportMods.log";

// Will become INI options
constexpr bool LOGGING_ENABLED = true;
constexpr auto FILE_NAME = "Modlist.htm";

// This will be where vortex.deployment.json resides.
char Game_Path[MAX_PATH];

// The output stream is global right now.
std::ofstream output;

// Where are we?
bool GetPath()
{
	return GetCurrentDirectoryA(MAX_PATH, Game_Path);
}

void LogLine(std::string line)
{
	if (!LOGGING_ENABLED)
	{
		return;
	}

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

/*
	Start the log and find the game folder.
	Only return false when the log can't be used.
// */
bool StartUp()
{
	// Start with the current folder as our path.
	bool bPath = GetPath();

	if (LOGGING_ENABLED)
	{
		// Find the Documents folder.
		TCHAR szFolderPath[MAX_PATH];
		if (!SHGetSpecialFolderPath(NULL, szFolderPath, CSIDL_MYDOCUMENTS, FALSE))
		{
			return false;
		}
		// Add the SFSE log path.
		std::wstring logPath (szFolderPath);
		logPath.append(LOG_PATH);

		// Open the log file
		output.open(logPath);
		// Stop if the log can't open.
		if (!output)
		{
			return false;
		}
	}

	// Stop in case there's no path.
	if (!bPath)
	{
		LogLine("Unable to determine path!");
		output.close();
		return true;
	}

	// Log the current folder
	std::string line("Started from \"");
	line.append(Game_Path);
	line.append("\".");
	LogLine(line);

	if (!strstr(Game_Path, "common\\Starfield"))
	{
		LogLine("Not started from the Starfield folder!");
		output.close();
		return true;
	}

	output.close();
	return true;
}
