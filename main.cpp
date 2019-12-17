#include <cstdlib>
#include <cstdio>
#include <string>
#include <map>
#include <regex>
#include <vector>
#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace std::string_literals;

// we don't have P1275 but this works well enough as a substitute for now
namespace std {
	std::vector<std::string const> arguments;
}

static const int kTabWidth = 4; // fight me.

static bool gColoredOutput = isatty(STDOUT_FILENO);
static const char* gColorBrightRed     = gColoredOutput ? "\u001b[31;1m" : "";
static const char* gColorBrightGreen   = gColoredOutput ? "\u001b[32;1m" : "";
static const char* gColorBrightMagenta = gColoredOutput ? "\u001b[35;1m" : "";
static const char* gColorBrightDefault = gColoredOutput ? "\u001b[39;1m" : "";
static const char* gColorReset         = gColoredOutput ? "\u001b[0m"    : "";

static std::map<std::string,int> gObservedFlags;

// I'd love to use std::filesystem the way god intended, but alas, Apple:
// https://twitter.com/lunasorcery/status/1205904118277197825
// https://twitter.com/jfbastien/status/1205906768313733120
static bool fileExists(const std::string& path)
{
	FILE* fh = fopen(path.c_str(), "rb");
	if (fh) {
		fclose(fh);
		return true;
	} else {
		return false;
	}
}

static std::string repeatChar(char c, size_t count)
{
	std::string buffer;
	buffer.resize(count);
	memset(buffer.data(), c ? c : ' ', count);
	return buffer;
}

static bool isFlag(const std::string& arg)
{
	return arg[0] == '-';
}

static std::vector<std::string> getFileLines(const std::string& path)
{
	std::vector<std::string> lines;
	std::ifstream fh(path);
	if (fh) {
		std::string line;
		while (getline(fh, line)) {
			lines.push_back(line);
		}
	}
	return lines;
}

static std::string replaceTabsWithSpaces(const std::string& str)
{
	const std::string tabAsSpaces = repeatChar(' ', kTabWidth);

	std::string result;
	for (const auto& c : str) {
		if (c == '\t') {
			result += tabAsSpaces;
		} else {
			result += c;
		}
	}
	return result;
}

static size_t computePositionAccountingForTabs(const std::string& str, size_t pos)
{
	size_t adjustedPosition = 0;
	for (size_t i = 0; i < pos; ++i) {
		if (str[i] == '\t') {
			adjustedPosition += kTabWidth;
		} else {
			adjustedPosition++;
		}
	}
	return adjustedPosition;
}

static void handleHelpFlag()
{
	for (const auto& arg : std::arguments) {
		if (arg == "-help" || arg == "--help" || arg == "--help-hidden") {
			printf("You will find no help here.\n");
			exit(0);
		}
	}
}

static void handleVersionFlag()
{
	for (const auto& arg : std::arguments) {
		if (arg == "-v" || arg == "--version") {
			printf("SUCC version 0.0.1\n");
			printf("Target: at least a couple dozen retweets and maybe a 'luna why'\n");
			printf("Thread model: none\n");
			printf("InstalledDir: /dev/circles-of-hell/9\n");
			exit(0);
		}
	}
}

static void checkFilesActuallyExist()
{
	bool hasAnyFiles = false;
	bool hasAnyMissingFiles = false;
	for (const auto& arg : std::arguments) {
		if (!isFlag(arg)) {
			if (fileExists(arg)) {
				hasAnyFiles = true;
			} else {
				hasAnyMissingFiles = true;
				fprintf(stderr, "succ: %serror:%s no such file or directory: '%s'\n", gColorBrightRed, gColorReset, arg.c_str());
			}
		}
	}
	if (!hasAnyFiles) {
		fprintf(stderr, "succ: %serror:%s no input files\n", gColorBrightRed, gColorReset);
		exit(1);
	} else if (hasAnyMissingFiles) {
		exit(1);
	}
}

static void handleDuplicateFlag(const std::string& flag)
{
	auto result = gObservedFlags.find(flag);
	if (result != gObservedFlags.end()) {
		switch (result->second) {
			case 1: {
				fprintf(stderr, "%swarning:%s Clearly you have very little confidence in your compiler, since you feel the need to specify the same flag twice. [%s]\n", gColorBrightMagenta, gColorReset, flag.c_str());
				break;
			}
			case 2: {
				fprintf(stderr, "%swarning:%s Again?! You really have no trust at all, do you? [%s]\n", gColorBrightMagenta, gColorReset, flag.c_str());
				break;
			}
			case 3: {
				fprintf(stderr, "%swarning:%s Goodness me. I refuse to believe you're this mistrustful. Go check your build scripts, you've surely got a mistake somewhere. [%s]\n", gColorBrightMagenta, gColorReset, flag.c_str());
				break;
			}
		}
		gObservedFlags[flag]++;
	} else {
		gObservedFlags.emplace(flag, 1);
	}
}

static void handleWarningFlag(const std::string& flag)
{
	if (flag.starts_with("-Wno-") || flag == "-w") {
		fprintf(stderr, "%swarning:%s Have confidence in your code. Do not try to disable warnings. [%s]\n", gColorBrightMagenta, gColorReset, flag.c_str());
		return;
	}

	if (flag == "-Werror" || flag.starts_with("-Werror=")) {
		fprintf(stderr, "succ: %serror:%s You asked for an error, so here, have one. [%s]\n", gColorBrightRed, gColorReset, flag.c_str());
		exit(1);
	}
}

static void handleFeatureFlag(const std::string& flag)
{
	// https://twitter.com/___srv_/status/1205722571100041216
	// https://twitter.com/Andrew_Taylor/status/1205764994526265345
	if (std::regex_search(flag, std::regex("^-fpwe+ase$"))) {
		usleep((flag.length() - 7) * 1000000);
		return;
	}

	if (flag == "-fmodules" || flag == "-fmodules-ts") {
		fprintf(stderr, "succ: %serror:%s Don't even bother. Modules are broken and you know it. [%s]\n", gColorBrightRed, gColorReset, flag.c_str());
		exit(1);
	}
}

static void handleStdFlag(const std::string& flag)
{
	const std::string version = flag.substr(5);

	if (std::regex_search(version, std::regex("^(c|gnu)\\+\\+(\\d+|2a)$"))) {
		if (version.starts_with("gnu")) {
			fprintf(stderr, "succ: %serror:%s SUCC only supports standards-compliant code. We'll have none of those fancy GNU extensions here, thank you. [%s]\n", gColorBrightRed, gColorReset, flag.c_str());
			exit(1);
		} else if (version == "c++98" || version == "c++03") {
			fprintf(stderr, "%swarning:%s Bring yourself into the modern day, gramps. [%s]\n", gColorBrightMagenta, gColorReset, flag.c_str());
			return;
		} else if (version == "c++23") {
			fprintf(stderr, "%swarning:%s Look, I want epochs as much as you do, but be patient. We're not there yet. [%s]\n", gColorBrightMagenta, gColorReset, flag.c_str());
			return;
		} else if (version == "c++11" || version == "c++14" || version == "c++17" || version == "c++2a") {
			// do nothing
			return;
		}
	}

	fprintf(stderr, "succ: %serror:%s What? That's not a real version of the language. [%s]\n", gColorBrightRed, gColorReset, flag.c_str());
	exit(1);
}

static void handleFlag(const std::string& flag)
{
	handleDuplicateFlag(flag);

	if (flag.starts_with("-W") || flag == "-w") {
		handleWarningFlag(flag);
	} else if (flag.starts_with("-f")) {
		handleFeatureFlag(flag);
	} else if (flag.starts_with("-std=")) {
		handleStdFlag(flag);
	}
}

static void handleFile(const std::string& file)
{
	const std::vector<std::string> lines = getFileLines(file);

	for (size_t i = 0; i < lines.size(); ++i) {
		const int lineNum = i+1;
		const std::string& line = lines[i];

		if (size_t pos = line.find("std::filesystem"); pos != std::string::npos) {
			fprintf(stderr, "%s%s:%d:%zu %serror:%s std::filesystem is unavailable: introduced in macOS 11.19%s\n", gColorBrightDefault, file.c_str(), lineNum, pos+1, gColorBrightRed, gColorBrightDefault, gColorReset);
			fprintf(stderr, "%s\n", replaceTabsWithSpaces(line).c_str());
			fprintf(stderr, "%s%s^%s%s\n", repeatChar(' ', computePositionAccountingForTabs(line, pos)).c_str(), gColorBrightGreen, repeatChar('~', 14).c_str(), gColorReset);
		}

		if (size_t pos = line.find("#include <C++>"); pos != std::string::npos) {
			fprintf(stderr, "%s%s:%d:%zu %sawesome:%s Heck yeah, inclusivity!%s\n", gColorBrightDefault, file.c_str(), lineNum, pos+1, gColorBrightGreen, gColorBrightDefault, gColorReset);
			fprintf(stderr, "%s\n", replaceTabsWithSpaces(line).c_str());
			fprintf(stderr, "%s%s^%s%s\n", repeatChar(' ', computePositionAccountingForTabs(line, pos)).c_str(), gColorBrightGreen, repeatChar('~', 13).c_str(), gColorReset);
		}
	}
}

static void attemptToBeFunny()
{
	for (const auto& arg : std::arguments) {
		if (isFlag(arg)) {
			handleFlag(arg);
		} else {
			handleFile(arg);
		}
	}
}

static void giveUpAndJustSayNo()
{
	fprintf(stderr, "succ: %serror:%s no\n", gColorBrightRed, gColorReset);
}

// [chanting] CHAR CONST STAR CONST STAR CONST
int main(int argc, char const * const * const argv)
{
	// yucky boilerplate I wish I didn't need
	// also, deliberately skip the first argument (process path)
	// though whether P1275 would do this is uncertain
	// https://twitter.com/slurpsmadrips/status/1205948788864507904
	for (int i = 1; i < argc; ++i) {
		std::arguments.push_back(argv[i]);
	}

	handleHelpFlag();
	handleVersionFlag();
	checkFilesActuallyExist();
	attemptToBeFunny();
	giveUpAndJustSayNo();
	return 1;
}
