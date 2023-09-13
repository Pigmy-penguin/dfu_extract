# Default compiler (clang if available, otherwise gcc)
CC := $(shell basename $$(command -v clang 2>/dev/null || echo gcc))
CFLAGS := -Wall -Wextra -Wpedantic -Iinclude -O2 -fstack-protector-strong -D_FORTIFY_SOURCE=2

# Source and object files
SRC_DIR := src
INCLUDE_DIR := include
OBJ_DIR := obj

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Executable name
TARGET := dfu_extract

# Default target
all: $(TARGET)
	@echo "Build complete."

# Compile each source file into an object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@echo "[$(CC)] Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create the executable
$(TARGET): $(OBJS)
	@echo "[$(CC)] Linking $@"
	@$(CC) $(CFLAGS) $^ -o $@

# Clean build artifacts
clean:
	@echo "Cleaning"
	@rm -rf $(OBJ_DIR) $(TARGET)

# Install the executable (you may need to adjust the destination)
install: $(TARGET)
	@echo "Installing $(TARGET) to /usr/local/bin/"
	@cp $(TARGET) /usr/local/bin/

# Verbose mode
ifeq ($(VERBOSE), 1)
CFLAGS += -DVERBOSE
endif

# Help target
help:
	@echo "Available targets:"
	@echo "  all (default)   - Build the executable"
	@echo "  install         - Install the executable to /usr/local/bin/"
	@echo "  clean           - Clean build artifacts"
	@echo "  help            - Display this help message"

# Phony targets
.PHONY: all clean install help