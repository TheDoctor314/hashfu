EXEC  ?= main
SRC_DIR := src
TEST_DIR := $(SRC_DIR)/tests

CFLAGS := -Wall -Wextra -Werror -Wpedantic -g --std=c++17

# Final build step
$(EXEC):	$(SRC_DIR)/main.cpp
		$(CXX) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
		rm $(EXEC)

RUNNER ?= $(TEST_DIR)/tests_runner
# Target to run tests
test: $(RUNNER)
		./$<

# Building the tests_runner
$(RUNNER): $(RUNNER).cpp
		$(CXX) -o $@ $<

# Generate tests_runner.cpp
$(RUNNER).cpp: $(TEST_DIR)/TestHashTable.h
		cxxtestgen -o $@ --error-printer $^
