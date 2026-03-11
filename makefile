# Source and target
SRCS   = main.cpp
TARGET = raylib_app

# Add CS3113 library files if they exist
ifeq ($(wildcard CS3113/cs3113.cpp),CS3113/cs3113.cpp)
    SRCS += CS3113/cs3113.cpp
endif

ifeq ($(wildcard CS3113/Entity.cpp),CS3113/Entity.cpp)
    SRCS += CS3113/Entity.cpp
endif

# OS detection
UNAME_S := $(shell uname -s)

CXX      = g++
CXXFLAGS = -std=c++11

# Raylib via pkg-config
RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
RAYLIB_LIBS   = $(shell pkg-config --libs   raylib)

ifeq ($(UNAME_S), Darwin)
    CXXFLAGS += -arch arm64 $(RAYLIB_CFLAGS)
    LIBS      = $(RAYLIB_LIBS) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    EXEC      = ./$(TARGET)
else ifneq (,$(findstring MINGW,$(UNAME_S)))
    CXXFLAGS += -IC:/raylib/include
    LIBS      = -LC:/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm
    TARGET   := $(TARGET).exe
    EXEC      = ./$(TARGET)
else
    LIBS  = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    EXEC  = ./$(TARGET)
endif

# Build rule
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

# Clean rule
clean:
	@if [ -f "$(TARGET)"     ]; then rm -f $(TARGET);     fi
	@if [ -f "$(TARGET).exe" ]; then rm -f $(TARGET).exe; fi

# Run rule
run: $(TARGET)
	$(EXEC)
