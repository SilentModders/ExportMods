/*
	SFSE Plugin to export mods deployed with Vortex.
	By Bit Mage
// */

#include "pch.h"
#include "Shlobj.h"
#include "operations.h"
#include <vector>
#include <algorithm>

constexpr auto LOG_PATH = "\\My Games\\Starfield\\SFSE\\Logs\\ExportMods.log"; // Where the log goes
constexpr auto VORTEX_FILE = "vortex.deployment.json"; // The name of the Vortex deployment file
constexpr auto MAX_LINES = 5000000; // How many lines is too many?

char Game_Path[MAX_PATH]; // This will be where vortex.deployment.json resides.

constexpr auto INI_PATH = "\\Data\\SFSE\\Plugins\\ExportMods.ini"; // Where the INI should be
bool bEnableLogging = false; // Is the log enabled in the INI?
char modReport[MAX_PATH]="Modlist.htm"; // Default file name, INI option

// The file streams are global.
std::ofstream output;// Log file
std::ifstream input; // Vortex file
std::ofstream report;// htm file

std::vector<std::string>Mods; // Full mod list

// Where are we?
bool GetPath()
{
	return GetCurrentDirectoryA(MAX_PATH, Game_Path);
}

/*
	Start the log and find the game folder.
	Only return false when the log can't be used.
// */
bool StartUp()
{
	// Start with the current folder as our path.
	bool bPath = GetPath();

	std::string iniPath(Game_Path);
	iniPath.append(INI_PATH);
	bEnableLogging = GetPrivateProfileIntA("Export", "iEnableLogging", bEnableLogging, iniPath.c_str());

	GetPrivateProfileStringA("Export", "sExportName", modReport, modReport, MAX_PATH, iniPath.c_str());

	// Find the Documents folder.
	char szFolderPath[MAX_PATH];
	if (!SHGetSpecialFolderPathA(NULL, szFolderPath, CSIDL_MYDOCUMENTS, FALSE))
	{
		return false;
	}

	if (bEnableLogging)
	{
		// Add the SFSE log path.
		std::string logPath(szFolderPath);
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

	// Open the export file
	std::string exportPath;
	exportPath.append(modReport);
	report.open(exportPath);
	// Stop if the file can't open.
	if (!report)
	{
		std::string err("Unable to open export file \"");
		err.append(exportPath);
		err.append("\"!");
		LogLine(err);
		input.close();
		output.close();
		return true;
	}

	// Parse the file.
	ReadMods();

	// Close up.
	input.close();
	report.close();
	output.close();
	return true;
}

// Add the next mod to the log.
// return the last word processed.
std::string LogMod()
{
	// Maximum length for mod name, in words
	const auto MAX_WORDS = 20;

	// Store the whole mod name before adding it to the file.
	std::string modname = "";

	// Load the next part of the input file into an empty string.
	std::string token = "";
	input >> token;

	// Log to file.
	for (int k = 0; k < MAX_WORDS; k++)
	{
		// Strip leading quotes.
		if (token[0] == '"')
		{
			token = token.substr(1, token.length() - 1);
		}
		// Strip trailing commas.
		if (token[token.length() - 1] == ',')
		{
			token = token.substr(0, token.length() - 1);
		}
		// Strip trailing quotes.
		if (token[token.length() - 1] == '"')
		{
			token = token.substr(0, token.length() - 1);
		}

		// Log that word and advance input to the next one.
		modname.append(token);
		input >> token;

		// The line under the mod has been reached. Break the loop.
		if (token == "\"target\":")
		{
			Mods.push_back(modname);
			//report << modname;
			//report << "<br>" << std::endl;
			break;
		}

		// Add a space between words, if the last one wasn't blank.
		if (token.length() > 1)
		{
			modname.append(" ");
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

	// Begin the HTML with a title and timestamp.
	report << "<!DOCTYPE html>" << std::endl;
	report << "<html lang=en>" << std::endl << "<head>" << std::endl;
	report << "<title>Automated Mod Export</title>" << std::endl;
	report << "</head>" << std::endl << "<body>" << std::endl;
	report << "<br>" << std::endl;
	report << "File generated ";
	report << TimeStamp() << std::endl;
	report << "<br><br>" << std::endl;

	std::string line = "";

	// Instantly toss out the beginning tokens.
	for (int i = 0; i < 21; i++)
	{
		input >> line;
	}

	int maxLines = MAX_LINES;
	while (line != "")
	{
		// Prevent the while loop from
		// potentially hogging the CPU.
		Sleep(0);

		// Find the next mod,
		// but peek at the file
		// to see if we're done.
		line = LogMod();

		// Make sure only a reasonable
		// amount of log is written.
		--maxLines;
		if (maxLines <= 0)
		{
			LogLine("Modlist too long!");
			break;
		}
	}
	if (maxLines > 0)
	{
		LogLine("Modlist complete!");
	}

	// The mod list needs to be sorted to remove duplicates.
	std::sort(Mods.begin(), Mods.end());
	auto last = std::unique(Mods.begin(), Mods.end());
	Mods.erase(last, Mods.end());

	// Output the shorter mod list.
	for (int j = 0; j < Mods.size(); j++)
	{
		report << Mods[j] << "<br>" << std::endl;
	}
	report << "<br>" << std::endl;

	// End the HTML.
	report << "</body>" << std::endl;
	report << "</html>" << std::endl;
}

void LogLine(std::string line)
{
	// Don't do anything if we're not logging.
	if (!bEnableLogging)
	{
		return;
	}

	// Start log lines with a timestamp.
	std::string stamp("[");
	stamp.append(TimeStamp());
	stamp.append("] ");

	output << stamp << line << std::endl;
}

std::string TimeStamp()
{
	// Get the time.
	time_t now = time(0);
	char sTime[26];
	ctime_s(sTime, 26, &now);

	// Remove extra line break.
	sTime[strlen(sTime) - 1] = '\0';

	return sTime;
}
