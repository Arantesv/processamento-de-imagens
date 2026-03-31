CXX := g++
TARGET := proj1
BUILD_DIR := build
SRC_DIR := src
INC_DIR := include

SDL3_PC := $(shell if pkg-config --exists sdl3; then echo sdl3; else echo SDL3; fi)
SDL3_IMAGE_PC := $(shell if pkg-config --exists sdl3-image; then echo sdl3-image; elif pkg-config --exists SDL3_image; then echo SDL3_image; else echo sdl3-image; fi)
SDL3_TTF_PC := $(shell if pkg-config --exists sdl3-ttf; then echo sdl3-ttf; elif pkg-config --exists SDL3_ttf; then echo SDL3_ttf; else echo sdl3-ttf; fi)

CXXFLAGS := -std=c++11 -Wall -Wextra -Wpedantic -O2 -I$(INC_DIR) \
	$(shell pkg-config --cflags $(SDL3_PC) $(SDL3_IMAGE_PC) $(SDL3_TTF_PC))
LDFLAGS := $(shell pkg-config --libs $(SDL3_PC) $(SDL3_IMAGE_PC) $(SDL3_TTF_PC))

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET) $(IMAGE)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
