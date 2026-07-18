CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude
LDLIBS   := -pthread

LIB  := bin/liblogger.so
APP  := bin/logger_app
TEST := bin/logger_tests

LIB_SRCS  := $(wildcard src/logger/*.cpp)
APP_SRCS  := $(wildcard app/*.cpp)
TEST_SRCS := $(wildcard tests/*.cpp)

LINK_LIB := -Lbin -llogger -Wl,-rpath,'$$ORIGIN'

.PHONY: all lib app test clean

all: lib app

lib: $(LIB)

app: $(APP)

$(LIB): $(LIB_SRCS)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -Isrc -fPIC -shared -o $@ $^ $(LDLIBS)

$(APP): $(APP_SRCS) $(LIB)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $(APP_SRCS) $(LINK_LIB) $(LDLIBS)

$(TEST): $(TEST_SRCS) $(LIB)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -Isrc -Iapp -o $@ $(TEST_SRCS) $(LINK_LIB) $(LDLIBS)

test: $(TEST)
	./$(TEST)

clean:
	rm -rf bin