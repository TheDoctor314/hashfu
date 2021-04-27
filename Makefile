EXEC  ?= main
SRC_DIR := src
TEST_DIR := tests

CFLAGS := -Wall -Wextra -Werror -Wpedantic -g --std=c++17

all: $(EXEC) test

# Final build step
$(EXEC):	$(SRC_DIR)/main.cpp
		$(CXX) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
		rm $(EXEC)
		rm $(RUNNER)

RUNNER ?= $(TEST_DIR)/tests_runner
# Target to run tests
test: $(RUNNER)
		./$<

# Building the tests_runner
$(RUNNER): $(RUNNER).cpp
		$(CXX) -I$(SRC_DIR) -o $@ $<

# Generate tests_runner.cpp
$(RUNNER).cpp: $(wildcard $(TEST_DIR)/*.h)
		cxxtestgen -o $@ --error-printer $^
