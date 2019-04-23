# Declaration of variables
CC = gcc
CFLAGS = -g -Wall -Wextra -Wunused
 
# Directory names
SOURCE_DIR = src
BUILD_DIR = build

# File names
EXEC = bank
SOURCES = $(wildcard $(SOURCE_DIR)/*.c)
OBJECTS =  $(patsubst $(SOURCE_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

dir_guard = @mkdir -p $(BUILD_DIR)
 
# Main target
$(EXEC): $(OBJECTS)
	$(dir_guard)
	$(CC) $(SOURCES) -o $(EXEC)
 
# To obtain object files
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(dir_guard)
	$(CC) -c $(CFLAGS) $< -o $@

# To remove generated files
clean:
	rm -f $(EXEC) $(OBJECTS)
	rmdir $(BUILD_DIR)