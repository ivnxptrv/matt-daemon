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

# Default Target
all: $(NAME)

# Linking the executable
$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $@

# Compiling objects (order-only deps create the dirs cleanly even after fclean)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR) $(DEP_DIR)
	$(CXX) $(CXXFLAGS) -MF $(DEP_DIR)/$*.d $(INCLUDE) -c $< -o $@

$(OBJ_DIR) $(DEP_DIR):
	mkdir -p $@

# Cleanup
clean:
	$(RM) $(OBJ_DIR) $(DEP_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

kill: 
	sudo kill -TERM $(pgrep Matt_daemon)

# Include dependencies
-include $(DEP)

.PHONY: all clean fclean re
