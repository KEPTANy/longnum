TARGET        := liblongnum.a

BUILD_DIR     := build
INCLUDE_DIR   := include
EXAMPLES_DIR  := examples
LIB_DIR       := lib
SRC_DIR       := src
TEST_DIR      := test

SRCS          := $(wildcard $(SRC_DIR)/*.cpp)
OBJS          := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS          := $(OBJS:.o=.d)

CXX           := clang++
CXXFLAGS      := -O2 -Wall -Wextra -std=c++20 -pedantic-errors
CPPFLAGS      := -MMD -MP -I$(INCLUDE_DIR)/
AR            := ar
ARFLAGS       := -r -c -s

TEST_SRCS     := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS     := $(TEST_SRCS:%.cpp=$(BUILD_DIR)/%.o)
TEST_FLAGS    := -I$(LIB_DIR)/doctest/doctest/ -DLONGNUM_TEST_PRIVATE
TEST_TARGET   := $(TEST_DIR)/test_runner

EXAMPLES_SRCS := $(wildcard $(EXAMPLES_DIR)/*.cpp)
EXAMPLES_OBJS := $(EXAMPLES_SRCS:%.cpp=$(BUILD_DIR)/%.o)
EXAMPLES      := $(EXAMPLES_SRCS:%.cpp=%)

RM            := rm -f 

.PHONY: all clean fclean re check-format test
.PRECIOUS: $(BUILD_DIR)/%.o $(BUILD_DIR)/%.d

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $< -c -o $@

$(BUILD_DIR)/$(TEST_DIR)%.o: $(TEST_DIR)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(TEST_FLAGS) $(CPPFLAGS) $< -c -o $@

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

all: $(TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(TARGET)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $^

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(EXAMPLES_DIR)/%: $(BUILD_DIR)/$(EXAMPLES_DIR)/%.o $(TARGET)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

examples: $(EXAMPLES)

clean:
	$(RM) -r $(BUILD_DIR)

fclean: clean
	$(RM) $(TARGET)
	$(RM) $(TEST_TARGET)
	$(RM) $(EXAMPLES)

re:
	$(MAKE) fclean
	$(MAKE) all

check-format:
	clang-format --dry-run --Werror \
		$(shell find $(SRC_DIR) $(INCLUDE_DIR) $(EXAMPLES_DIR) \
			-name '*.cpp' -o -name '*.hpp' -o -name '*.h')
