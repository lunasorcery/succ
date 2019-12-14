#include <cstdlib>
#include <cstdio>
#include <string>
#include <unistd.h>

using namespace std::string_literals;

// this whole cursed thing would at least be a bit more elegant if we had P1275

static bool gColoredOutput = isatty(STDOUT_FILENO);
static const char* gColorBrightRed = gColoredOutput ? "\u001b[31;1m" : "";
static const char* gColorReset     = gColoredOutput ? "\u001b[0m"    : "";

static bool isFlag(char const * const arg)
{
	return arg[0] == '-';
}

// [chanting] CONST STAR CONST STAR CONST...
static void handleHelpFlag(const int argc, char const * const * const argv)
{
	for (int i = 1; i < argc; ++i) {
		if (argv[i] == "-help"s || argv[i] == "--help"s || argv[i] == "--help-hidden"s) {
			printf("You will find no help here.\n");
			exit(0);
		}
	}
}

static void handleVersionFlag(const int argc, char const * const * const argv)
{
	for (int i = 1; i < argc; ++i) {
		if (argv[i] == "-v"s || argv[i] == "--version"s) {
			printf("SUCC version 0.0.1\n");
			printf("Target: at least a couple dozen retweets and maybe a 'luna why'\n");
			printf("Thread model: none\n");
			printf("InstalledDir: /dev/circles-of-hell/9\n");
			exit(0);
		}
	}
}

static void checkFilesActuallyExist(const int argc, char const * const * const argv)
{
	bool hasAnyFiles = false;
	bool hasAnyMissingFiles = false;
	for (int i = 1; i < argc; ++i) {
		if (!isFlag(argv[i])) {
			FILE* fh = fopen(argv[i], "rb");
			if (fh) {
				hasAnyFiles = true;
				fclose(fh);
			} else {
				hasAnyMissingFiles = true;
				fprintf(stderr, "succ: %serror:%s no such file or directory: '%s'\n", gColorBrightRed, gColorReset, argv[i]);
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

static void handleFlag(const std::string& flag)
{
	(void)flag;
	// TODO: add snarky responses to various common flags
}

static void handleFile(const std::string& file)
{
	(void)file;
	// TODO: add minimal "parsing" of code to allow for basic shit-talking
}

static void attemptToBeFunny(const int argc, char const * const * const argv)
{
	for (int i = 1; i < argc; ++i) {
		if (isFlag(argv[i])) {
			handleFlag(argv[i]);
		} else {
			handleFile(argv[i]);
		}
	}
}

static void giveUpAndJustSayNo()
{
	printf("no\n");
}

int main(int argc, char const * const * const argv)
{
	handleHelpFlag(argc, argv);
	handleVersionFlag(argc, argv);
	checkFilesActuallyExist(argc, argv);
	attemptToBeFunny(argc, argv);
	giveUpAndJustSayNo();
	return 1;
}
