#include <log.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <dfu.h>

#define MAX_FILENAME_LENGTH 256

// Function to display usage information
void displayUsage() {
    printf("Usage: dfu_extract [options] <input_file>\n");
    printf("Options:\n");
    printf("  -o, --output <output_file>   Specify the output file (default: output.bin)\n");
    printf("  -h, --help                   Display this help message\n");
}

int main(int argc, char* argv[]) {
    // Print a beautiful banner
    printf("DFU Extraction Tool\n");
    printf("----------------------------------------\n" ANSI_COLOR_RESET);

    char inputFile[MAX_FILENAME_LENGTH] = "";
    char outputFile[MAX_FILENAME_LENGTH] = "output.bin";

    int opt;
    int helpFlag = 0;

    struct option long_options[] = {
        {"output", required_argument, NULL, 'o'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    // Define the options using getopt_long
    while ((opt = getopt_long(argc, argv, "o:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'o':
                strncpy(outputFile, optarg, MAX_FILENAME_LENGTH - 1);
                outputFile[MAX_FILENAME_LENGTH - 1] = '\0'; // Ensure null-terminated
                break;
            case 'h':
                helpFlag = 1;
                break;
            default:
                displayUsage();
                return 1;
        }
    }

    // Check if the help flag is set
    if (helpFlag) {
        displayUsage();
        return 0;
    }

    // Check for non-option arguments (the input file)
    if (optind < argc) {
        strncpy(inputFile, argv[optind], MAX_FILENAME_LENGTH - 1);
        inputFile[MAX_FILENAME_LENGTH - 1] = '\0'; // Ensure null-terminated
    } else {
        logMessage(ANSI_COLOR_RED, "Error: Input file not specified.");
        displayUsage();
        return 1;
    }

    // Perform DFU extraction
    if (extractDFU(inputFile, outputFile) != 0) {
        logMessage(ANSI_COLOR_RED, "DFU extraction failed.");
        return 1;
    }

    return 0;
}
