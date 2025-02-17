TARGET      := liblongnum.a

BUILD_DIR   := build
INCLUDE_DIR := include
LIB_DIR     := lib
SRC_DIR     := src
TEST_DIR    := test

SRCS        := $(wildcard $(SRC_DIR)/*.cpp)
OBJS        := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)

CXX         := clang++
CXXFLAGS    := -O2 -Wall -Wextra -std=c++20 -pedantic-errors
CPPFLAGS    := -MMD -MP -I$(INCLUDE_DIR)/
AR          := ar
ARFLAGS     := -r -c -s

TEST_SRCS   := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS   := $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TEST_FLAGS  := -I$(LIB_DIR)/doctest/doctest/
TEST_TARGET := test_runner

RM          := rm -f 

.PHONY: all clean fclean re check-format test

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $< -c -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(TEST_FLAGS) $(CPPFLAGS) $< -c -o $@

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

all: $(TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(TARGET)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	$(RM) -r $(BUILD_DIR)

fclean: clean
	$(RM) $(TARGET)
	$(RM) $(TEST_TARGET)

re:
	$(MAKE) fclean
	$(MAKE) all

check-format:
	clang-format --dry-run --Werror \
		$(shell find $(SRC_DIR) $(INCLUDE_DIR) \
			-name '*.cpp' -o -name '*.hpp' -o -name '*.h')
