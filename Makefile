# Compiler and Flags
CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -MMD
INCLUDE_DIRS := ./include/
INCLUDE := $(addprefix -I,$(INCLUDE_DIRS))
RM := rm -rf

# Project Name
NAME := Matt_daemon

# Directories
SRC_DIR := src
OBJ_DIR := obj
DEP_DIR := dep

# Files
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))
DEP := $(patsubst $(SRC_DIR)/%.cpp, $(DEP_DIR)/%.d, $(SRC))

# Ensure directories exist before compiling
$(shell mkdir -p $(OBJ_DIR) $(DEP_DIR))

# Default Target
all: $(NAME)

# Linking the executable
$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $@

# Compiling objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -MF $(DEP_DIR)/$*.d $(INCLUDE) -c $< -o $@

# Cleanup
clean:
	$(RM) $(OBJ_DIR) $(DEP_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

# Include dependencies
-include $(DEP)

.PHONY: all clean fclean re
