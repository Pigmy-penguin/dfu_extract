#ifndef LOG_H
#define LOG_H

// Define text colors for logging
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[92m"
#define ANSI_COLOR_CYAN "\x1b[96m"

// Function to print log messages with color and timestamp
void logMessage(const char* color, const char* format, ...);

#endif
