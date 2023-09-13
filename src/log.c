#include <log.h>
#include <time.h>
#include <stdio.h>

// Function to print log messages with color and timestamp
void logMessage(const char* color, const char* format, ...) {
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    // Print the timestamp and color
    printf("[%02d:%02d:%02d] %s", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, color);

    // Use variadic arguments to handle formatted message
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    // Reset color
    printf("%s\n", ANSI_COLOR_RESET);
}
