#pragma once

// Returns false for unhandled error.
bool StartUp();

// Adds the passed string to the log with a timestamp.
void LogLine(std::string line);

// Reads the mod list.
void ReadMods();

// Returns the current date and time.
std::string TimeStamp();

// Guesses Mod Nexus' link for a mod
// Returns an empty string when it fails.
std::string GuessLinkID(std::string mod);
