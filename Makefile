NAME        := liblongnum.a

SRC_DIR     := src
INCLUDE_DIR := include
SRCS        := longnum.cpp
SRCS        := $(SRCS:%=$(SRC_DIR)/%)

BUILD_DIR   := build
OBJS        := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)

CXX         := g++
CXXFLAGS    := -O2 -Wall -Wextra -Werror -std=c++20 -pedantic-errors
CPPFLAGS    := -MMD -MP -I$(INCLUDE_DIR)
AR          := ar
ARFLAGS     := -r -c -s

RM          := rm -f 

.PHONY: all clean fclean re check-format

all: $(NAME)

$(NAME): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -c -o $@

clean:
	$(RM) -r $(BUILD_DIR)

fclean: clean
	$(RM) $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

check-format:
	clang-format --dry-run --Werror \
		$(shell find . -name '*.cpp' -o -name '*.hpp' -o -name '*.h')

.SILENT:
