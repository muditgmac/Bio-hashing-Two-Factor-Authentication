CC := clang

CFLAGS := \
	-std=c11 \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wconversion \
	-Wshadow \
	-Werror \
	-Iinclude

SANITIZERS := -fsanitize=address,undefined -fno-omit-frame-pointer

BUILD_DIR := build

BBS_SOURCE := src/bbs.c
BBS_TEST := tests/test_bbs.c
BBS_TEST_BINARY := $(BUILD_DIR)/test_bbs

.PHONY: all test test-sanitize clean

all: test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BBS_TEST_BINARY): $(BBS_SOURCE) $(BBS_TEST) include/bbs.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(BBS_SOURCE) $(BBS_TEST) -o $(BBS_TEST_BINARY)

test: $(BBS_TEST_BINARY)
	./$(BBS_TEST_BINARY)

test-sanitize: | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(BBS_SOURCE) $(BBS_TEST) \
		-o $(BUILD_DIR)/test_bbs_sanitize
	./$(BUILD_DIR)/test_bbs_sanitize

clean:
	rm -rf $(BUILD_DIR)
