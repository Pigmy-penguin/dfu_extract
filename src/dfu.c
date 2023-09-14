#include <log.h>
#include <dfu.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

const char* formatBytes(long long bytes) {
    static const char* suffix[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
    int index = 0;
    double size = (double)bytes;

    while (size >= 1024 && index < sizeof(suffix) / sizeof(suffix[0]) - 1) {
        size /= 1024;
        index++;
    }

    static char formatted[32];
    snprintf(formatted, sizeof(formatted), "%.2f %s", size, suffix[index]);
    return formatted;
}

char *string_repeat(int n, const char * s) {
  size_t slen = strlen(s);
  char * dest = malloc(n*slen+1);

  int i; char * p;
  for ( i=0, p = dest; i < n; ++i, p += slen ) {
    memcpy(p, s, slen);
  }
  *p = '\0';
  return dest;
}

static int keepGoing() {
loop:
    printf("Do you want to extract the images ? [yes/no] \033[1;33m> \033[0m");
    char input[10];
    while(1) {
        if (fgets(input, sizeof(input), stdin) != NULL) {
            input[strcspn(input, "\n")] = '\0'; // Remove newline character

            // Convert input to lowercase for case-insensitive comparison
            for (int i = 0; input[i]; i++) {
                input[i] = tolower(input[i]);
            }
            if (strcmp(input, "y") == 0 || strcmp(input, "yes") == 0) {
                return 1;
            } else if (strcmp(input, "n") == 0 || strcmp(input, "no") == 0) {
                return 0;
            } else {
                goto loop;
            }
        } else {
            logMessage(ANSI_COLOR_RED, "Error reading input.\n");
            return -1;
        }
    }
}

int findDFUSuffix(FILE *file, DFUSuffix *suffix) {
    size_t suffixSize = sizeof(DFUSuffix);

    fseek(file, 0, SEEK_END); // seek to end of file
    size_t fsize = ftell(file); // get current file pointer
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(fsize);
    if (buffer != NULL && fread(buffer, 1, fsize, file) == fsize) {
        const char *end = buffer + fsize - 2;
        const char *p = buffer;
        while (p < end) {
            if (p[0] == 'U' && p[1] == 'F' && p[2] == 'D') {
                memcpy(suffix, p-sizeof(uint64_t), sizeof(DFUSuffix));
                free(buffer);
                return 0;
            }
        p++;
        }
    }

    free(buffer);
    return -1;
}

// Function to extract DFU data
int extractDFU(const char* inputFile, const char* outputFile) {
    logMessage(ANSI_COLOR_RESET, "Extracting DFU data...");

    // Open input file
    FILE* inFile = fopen(inputFile, "rb");
    if (inFile == NULL) {
        logMessage(ANSI_COLOR_RED, "Error opening input file");
        return 1;
    }

    DFUPrefix prefix;
    size_t bytesRead = fread(&prefix, 1, sizeof(prefix), inFile);

    if (bytesRead != sizeof(prefix)) {
        if (feof(inFile)) {
            logMessage(ANSI_COLOR_RED, "Error: End of file reached before reading complete data");
        } else if (ferror(inFile)) {
            logMessage(ANSI_COLOR_RED, "Error reading file");
        }

        fclose(inFile); // Close the file before exiting
        return 1;
    }

    if (memcmp(prefix.szSignature, "DfuSe", 4) != 0) {
        logMessage(ANSI_COLOR_RED, "Wrong file signature");
        return 1;
    }

    logMessage(ANSI_COLOR_RESET, "DFU format revision: %d", prefix.bVersion);
    if (prefix.bVersion != 1) {
        logMessage(ANSI_COLOR_RED, "Unknown DFU format revision");
        return 1;
    }
    
    logMessage(ANSI_COLOR_RESET, "File size: %s", formatBytes(prefix.DFUImageSize));
    DFUSuffix *suffix = malloc(sizeof(DFUSuffix));
    if (suffix != NULL && findDFUSuffix(inFile, suffix) == 0) {
        // Successfully found and read the DFUSuffix structure
        if ((((uint16_t)suffix->bcdDFUHi << 8) | suffix->bcdDFULo) != 0x011A) {
            logMessage(ANSI_COLOR_RED, "Unknown DFU version");
            return -1;
        }
        logMessage(ANSI_COLOR_RESET, "Product ID: %d", ((uint16_t)suffix->idProductHi << 8) | suffix->idProductLo);
        logMessage(ANSI_COLOR_RESET, "Vendor ID: %d", ((uint16_t)suffix->idVendorHi << 8) | suffix->idVendorLo);
    } else {
        logMessage(ANSI_COLOR_RED, "Unable to find suffix. Invalid file");
        return -1;
    }

    logMessage(ANSI_COLOR_RESET, "%d images were found", prefix.bTargets);
    puts("----------------------------------------");
    for (int i = 0; i != prefix.bTargets; i++) {
        size_t dfuOffset = sizeof(DFUPrefix);
        size_t targetSize = sizeof(TargetPrefix);
        TargetPrefix target;

        // Seek to the offset for TargetPrefix and read the data
        if (fseek(inFile, dfuOffset, SEEK_SET) == -1 ||
        fread(&target, 1, targetSize, inFile) != targetSize) {
            logMessage(ANSI_COLOR_RED, "Error reading file");
            close(inFile);
            return 1;
        }
        if (memcmp(target.szSignature, "Target", 6) != 0) {
            logMessage(ANSI_COLOR_RED, "Wrong target signature");
            return 1;
        }
        if (target.bTargetNamed == 1) {
            logMessage(ANSI_COLOR_RESET, "Image %d : %s", i, target.szTargetName);
        }
        else {
            logMessage(ANSI_COLOR_RESET, "Image %d", i);
        }

        logMessage(ANSI_COLOR_RESET, "Size %s", formatBytes(target.dwTargetSize/10));
        logMessage(ANSI_COLOR_RESET, "%d elements", target.dwNbElements);

        fseek(inFile, dfuOffset+targetSize, SEEK_SET);

        for (int j = 0; j != target.dwNbElements+1; j++) {
            printf("Pos1: %d\n", ftell(inFile));
            ImageElement element;

            if (fread(&element, 1, sizeof(ImageElement), inFile) != sizeof(ImageElement)) {
                logMessage(ANSI_COLOR_RED, "Error reading file");
                close(inFile);
                return 1;
            }
            printf("Pos2: %d\n", ftell(inFile));
            logMessage(ANSI_COLOR_RESET, "--------------------");
            logMessage(ANSI_COLOR_RESET, "║ Element %d", j+1);
            logMessage(ANSI_COLOR_RESET, "║ Adress: 0x%08X", element.dwElementAddress);
            logMessage(ANSI_COLOR_RESET, "║ Size: %s", formatBytes(element.dwElementSize));

            char filename[255];
            snprintf(filename, sizeof(filename), "element-%d", j);
            FILE *binary = fopen(filename, "wb");
            fwrite((&(element.dwElementAddress))+sizeof(ImageElement), element.dwElementSize, 1, binary);
            fseek(inFile, ftell(inFile)+element.dwElementSize, SEEK_SET);
            printf("Pos3: %d\n", ftell(inFile));
        }

    }
    puts("----------------------------------------\n");

    if(keepGoing() == 1) {
        putchar('\n');
        logMessage(ANSI_COLOR_RESET, "Extracting images");
    }
    else {
        return 0;
    }

    // Open output file
    FILE* outFile = fopen(outputFile, "wb");
    if (outFile == NULL) {
        logMessage(ANSI_COLOR_RED, "Error opening output file");
        fclose(inFile);
        return 1;
    }

    //TODO: output

    // Close files
    if (fclose(inFile) == EOF) {
        logMessage(ANSI_COLOR_RED, "Error closing input file");
        return 1;
    }
    if (fclose(outFile) == EOF) {
        logMessage(ANSI_COLOR_RED, "Error closing output file");
        return 1;
    }
    logMessage(ANSI_COLOR_RESET, "DFU extraction complete.");
    return 0;
}
