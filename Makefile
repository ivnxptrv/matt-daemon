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

log:
	sudo tail -f /var/log/matt_daemon/matt_daemon.log

# ── Tests ────────────────────────────────────────────────────────────────
# Test binary links every production .o except main.o (doctest supplies main).
# Tests live in tests/, get their own obj_test/ + dep_test/ dirs.
TEST_DIR := tests
TEST_OBJ_DIR := obj_test
TEST_DEP_DIR := dep_test
TEST_INCLUDE := -I./include/ -I./tests/
TEST_SRC := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJ := $(patsubst $(TEST_DIR)/%.cpp, $(TEST_OBJ_DIR)/%.o, $(TEST_SRC))
TEST_DEP := $(patsubst $(TEST_DIR)/%.cpp, $(TEST_DEP_DIR)/%.d, $(TEST_SRC))
TEST_PROD_OBJ := $(filter-out $(OBJ_DIR)/main.o, $(OBJ))
TEST_BIN := test_runner

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJ) $(TEST_PROD_OBJ)
	$(CXX) $(TEST_OBJ) $(TEST_PROD_OBJ) -o $@

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp | $(TEST_OBJ_DIR) $(TEST_DEP_DIR)
	$(CXX) $(CXXFLAGS) -MF $(TEST_DEP_DIR)/$*.d $(TEST_INCLUDE) -c $< -o $@

$(TEST_OBJ_DIR) $(TEST_DEP_DIR):
	mkdir -p $@

clean_test:
	$(RM) $(TEST_OBJ_DIR) $(TEST_DEP_DIR) $(TEST_BIN)

# Hook test cleanup into fclean.
fclean: clean_test

# Include dependencies
-include $(DEP)
-include $(TEST_DEP)

.PHONY: all clean fclean re test clean_test
