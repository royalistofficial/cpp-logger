# Сборка библиотеки, приложения и тестов.
#
# Цели:
#   make lib    — только динамическая библиотека
#   make app    — приложение (тянет за собой библиотеку)
#   make test   — собрать и запустить тесты
#   make clean  — удалить все продукты сборки

CXX      := g++
WARNINGS := -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast
CXXFLAGS := -std=c++17 -O2 $(WARNINGS) -MMD -MP
LDLIBS   := -pthread

BUILD := build
BIN   := bin

LIB  := $(BIN)/liblogger.so
APP  := $(BIN)/logger_app
TEST := $(BIN)/logger_tests

LIB_SRCS     := $(wildcard src/logger/*.cpp)
APP_SRCS     := $(wildcard app/*.cpp)
APP_LIB_SRCS := $(filter-out app/main.cpp,$(APP_SRCS))
TEST_SRCS    := $(wildcard tests/*.cpp)

LIB_OBJS      := $(LIB_SRCS:%.cpp=$(BUILD)/lib/%.o)
APP_OBJS      := $(APP_SRCS:%.cpp=$(BUILD)/app/%.o)
APP_LIB_OBJS  := $(APP_LIB_SRCS:%.cpp=$(BUILD)/app/%.o)
TEST_OBJS     := $(TEST_SRCS:%.cpp=$(BUILD)/test/%.o)
# Тесты проверяют в том числе приватные классы библиотеки, которые не
# экспортируются из .so, поэтому собираются с исходниками библиотеки напрямую.
TEST_LIB_OBJS := $(LIB_SRCS:%.cpp=$(BUILD)/test/%.o)

# Приложение и тесты линкуются с библиотекой динамически. $ORIGIN позволяет
# запускать бинарник без установки библиотеки в систему и без LD_LIBRARY_PATH.
LINK_LIB := -L$(BIN) -llogger -Wl,-rpath,'$$ORIGIN'

.PHONY: all lib app test clean

all: lib app

lib: $(LIB)

app: $(APP)

# Символы библиотеки скрыты по умолчанию: наружу выходит только помеченное
# LOGGER_API.
$(BUILD)/lib/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -Iinclude -Isrc -fPIC -fvisibility=hidden \
		-fvisibility-inlines-hidden -c $< -o $@

$(BUILD)/app/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -Iinclude -Iapp -c $< -o $@

# Тестам доступны приватные заголовки библиотеки: часть проверок обращается
# к реализациям напрямую.
$(BUILD)/test/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -Iinclude -Isrc -Iapp -c $< -o $@

$(LIB): $(LIB_OBJS)
	@mkdir -p $(BIN)
	$(CXX) -shared -o $@ $^ $(LDLIBS)

$(APP): $(APP_OBJS) $(LIB)
	@mkdir -p $(BIN)
	$(CXX) -o $@ $(APP_OBJS) $(LINK_LIB) $(LDLIBS)

$(TEST): $(TEST_OBJS) $(TEST_LIB_OBJS) $(APP_LIB_OBJS)
	@mkdir -p $(BIN)
	$(CXX) -o $@ $(TEST_OBJS) $(TEST_LIB_OBJS) $(APP_LIB_OBJS) $(LDLIBS)

test: $(TEST)
	./$(TEST)

clean:
	rm -rf $(BUILD) $(BIN)

-include $(LIB_OBJS:.o=.d) $(APP_OBJS:.o=.d) $(TEST_OBJS:.o=.d) \
         $(TEST_LIB_OBJS:.o=.d)
