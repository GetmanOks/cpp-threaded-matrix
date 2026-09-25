CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pthread

# На Windows -static кладёт библиотеки внутрь exe, отдельно таскать их не нужно.
# На Linux статическая libstdc++ часто не установлена, поэтому там флаг не ставим.
ifeq ($(OS),Windows_NT)
	CXXFLAGS += -static
	EXE := .exe
	RM := del /Q
	RUN :=
	SHELL := cmd.exe
	.SHELLFLAGS := /c
	# Каталог MinGW должен быть первым в PATH.
	# Иначе компилятор подхватывает чужие libstdc++ и libwinpthread
	# (например из TestDisk или platform-tools) и молча падает.
	PATH := C:/msys64/ucrt64/bin;$(PATH)
	export PATH
else
	EXE :=
	RM := rm -f
	RUN := ./
endif

APP_SOURCES := main.cpp matrix.cpp multiply.cpp protocol.cpp
TEST_SOURCES := test_matrix.cpp matrix.cpp multiply.cpp protocol.cpp
HEADERS := matrix.hpp multiply.hpp protocol.hpp pause.hpp

APP := matrix$(EXE)
TEST_BIN := test_matrix$(EXE)

.PHONY: all test clean

all: $(APP)

ifeq ($(OS),Windows_NT)
$(APP): $(APP_SOURCES) $(HEADERS) build.cmd
	cmd /c build.cmd $(CXXFLAGS) $(APP_SOURCES) -o $(APP)

$(TEST_BIN): $(TEST_SOURCES) $(HEADERS) build.cmd
	cmd /c build.cmd $(CXXFLAGS) $(TEST_SOURCES) -o $(TEST_BIN)
else
$(APP): $(APP_SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(APP_SOURCES) -o $(APP)

$(TEST_BIN): $(TEST_SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) -o $(TEST_BIN)
endif

test: $(TEST_BIN)
	$(RUN)$(TEST_BIN)

clean:
ifeq ($(OS),Windows_NT)
	-taskkill /F /IM matrix.exe >NUL 2>&1 & exit /b 0
	-taskkill /F /IM test_matrix.exe >NUL 2>&1 & exit /b 0
endif
	-$(RM) $(APP) $(TEST_BIN) protocol.txt protocol-test.txt build-log.txt compile-out.txt
