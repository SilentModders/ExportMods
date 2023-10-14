// Main file for export

#include <cstdlib>
#include "pch.h"
#include "operations.h"
#include "Shlobj.h"

constexpr auto LOG_PATH = L"\\My Games\\Starfield\\SFSE\\Logs\\ExportMods.log"; // Where the log goes
constexpr auto VORTEX_FILE = "vortex.deployment.json"; // The name of the Vortex deployment file
constexpr auto MAX_LINES = 5000000; // How many lines is too many?

// Will become INI options
constexpr bool LOGGING_ENABLED = true; // Enable or disable the log.
constexpr auto FILE_NAME = "Modlist.htm"; // Filename of the generated modlist

// This will be where vortex.deployment.json resides.
char Game_Path[MAX_PATH];

// The file streams is global right now.
std::ofstream output;
std::ifstream input;

// Where are we?
bool GetPath()
{
	return GetCurrentDirectoryA(MAX_PATH, Game_Path);
}

// Adds the passed string to the log with a timestamp.
void LogLine(std::string line);

// Reads the mod list.
void ReadMods();

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

	// Log the current folder.
	std::string line("Started from \"");
	line.append(Game_Path);
	line.append("\".");
	LogLine(line);

	// Stop if this is running from elsewhere.
	if (!strstr(Game_Path, "common\\Starfield"))
	{
		LogLine("Not started from the Starfield folder!");
		output.close();
		return true;
	}

	// Open Vortex JSON.
	std::string dFile(Game_Path);
	dFile.append("\\");
	dFile.append(VORTEX_FILE);
	input.open(dFile);

	// Stop if the JSON can't open.
	if (!input)
	{
		std::string err("Unable to open ");
		err.append(VORTEX_FILE);
		err.append("!");
		LogLine(err);
		output.close();
		return true;
	}

	// Parse the file.
	ReadMods();

	// Close up.
	input.close();
	output.close();
	return true;
}

// Add the next mod to the log.
// return the last word processed.
std::string LogMod()
{
	// Maximum length for mod name, in words
	const auto MAX_WORDS = 20;

	// Load the next part of the input file to an empty string.
	std::string token = "";
	input >> token;

	// Log to file for now.
	for (int k = 0; k < MAX_WORDS; k++)
	{
		// Strip leading qoutes.
		if (token[0] == '"')
		{
			token = token.substr(1, token.length() - 1);
		}
		// Strip trailing commas.
		if (token[token.length() - 1] == ',')
		{
			token = token.substr(0, token.length() - 1);
		}
		// Strip trailing qoutes.
		if (token[token.length() - 1] == '"')
		{
			token = token.substr(0, token.length() - 1);
		}

		// Log that word and advance input to the next one.
		output << token;
		input >> token;

		// The line under the mod has been reached. Break the loop.
		if (token == "\"target\":")
		{
			output << std::endl;
			break;
		}

		// Add a space between words, if the last one wasn't blank.
		if (token.length() > 1)
		{
			output << " ";
		}
	}

	// Advance input until you find another mod.
	for (int l = 0; l < MAX_WORDS; l++)
	{
		input >> token;
		if (token == "\"source\":")
		{
			break;
		}
	}

	// Return the last word, to show our work.
	return token;
}

void ReadMods()
{
	LogLine("Listing mods:");

	std::string line = "";

	// Instantly toss out the first tokens.
	for (int i = 0; i < 21; i++)
	{
		input >> line;
	}

	int maxLines = MAX_LINES;
	while (line != "")
	{
		Sleep(0);
		line = LogMod();
		--maxLines;
		if (maxLines <= 0)
		{
			LogLine("Modlist too long!");
			break;
		}
	}
	LogLine("Modlist complete!");
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
