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

char *string_repeat( int n, const char * s ) {
  size_t slen = strlen(s);
  char * dest = malloc(n*slen+1);

  int i; char * p;
  for ( i=0, p = dest; i < n; ++i, p += slen ) {
    memcpy(p, s, slen);
  }
  *p = '\0';
  return dest;
}

// Function to extract DFU data
int extractDFU(const char* inputFile, const char* outputFile) {
    logMessage(ANSI_COLOR_GREEN, "Extracting DFU data...");

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

    logMessage(ANSI_COLOR_GREEN, "DFU format revision: %d", prefix.bVersion);
    if (prefix.bVersion != 1) {
        logMessage(ANSI_COLOR_RED, "Unknown DFU format revision");
        return 1;
    }
    
    logMessage(ANSI_COLOR_GREEN, "File size: %s", formatBytes(prefix.DFUImageSize));
    logMessage(ANSI_COLOR_GREEN, "Images: %d", prefix.bTargets);
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
        
        char buffer[300];
        snprintf(buffer, sizeof(buffer), "║ -- Image %d : %s", i, target.szTargetName);
        size_t l0 = strlen(buffer);
        logMessage(ANSI_COLOR_GREEN, "╔═%s%s", string_repeat(l0, "═"), "╗");
        if (target.bTargetNamed == 1) {
            logMessage(ANSI_COLOR_GREEN, "║ -- Image %d : %s", i, target.szTargetName, string_repeat((l0+4, " "), "║"));
        }
        else {
            logMessage(ANSI_COLOR_GREEN, "║ -- Image %d", i);
        }

        snprintf(buffer, sizeof(buffer), "║ -- Size %s", formatBytes(target.dwTargetSize/10));
        size_t l1 = strlen(buffer);
        snprintf(buffer, sizeof(buffer), "║ -- Elements %d", target.dwNbElements);
        size_t l2 = strlen(buffer);

        logMessage(ANSI_COLOR_GREEN, "║ -- Size %s%s%s", formatBytes(target.dwTargetSize/10), string_repeat((l0+4)-l1, " "), "║");
        logMessage(ANSI_COLOR_GREEN, "║ -- Elements %d", target.dwNbElements);


        for (int j = 0; j != target.dwNbElements; j++) {
            ImageElement element;
            if (fseek(inFile, dfuOffset+targetSize, SEEK_SET) == -1 ||
            fread(&element, 1, sizeof(ImageElement), inFile) != sizeof(ImageElement)) {
                logMessage(ANSI_COLOR_RED, "Error reading file");
                close(inFile);
                return 1;
        }
        }

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

    logMessage(ANSI_COLOR_GREEN, "DFU extraction complete.");
    return 0;
}
