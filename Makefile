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
BBS_SANITIZE_BINARY := $(BUILD_DIR)/test_bbs_sanitize

GRAM_SCHMIDT_SOURCE := src/gram_schmidt.c
GRAM_SCHMIDT_TEST := tests/test_gram_schmidt.c
GRAM_SCHMIDT_TEST_BINARY := $(BUILD_DIR)/test_gram_schmidt
GRAM_SCHMIDT_SANITIZE_BINARY := $(BUILD_DIR)/test_gram_schmidt_sanitize

.PHONY: all test test-bbs test-gram-schmidt test-sanitize clean

all: test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BBS_TEST_BINARY): $(BBS_SOURCE) $(BBS_TEST) include/bbs.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(BBS_SOURCE) $(BBS_TEST) -o $(BBS_TEST_BINARY)

$(GRAM_SCHMIDT_TEST_BINARY): $(GRAM_SCHMIDT_SOURCE) $(GRAM_SCHMIDT_TEST) include/gram_schmidt.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(GRAM_SCHMIDT_SOURCE) $(GRAM_SCHMIDT_TEST) -lm -o $(GRAM_SCHMIDT_TEST_BINARY)

test-bbs: $(BBS_TEST_BINARY)
	./$(BBS_TEST_BINARY)

test-gram-schmidt: $(GRAM_SCHMIDT_TEST_BINARY)
	./$(GRAM_SCHMIDT_TEST_BINARY)

test: test-bbs test-gram-schmidt

test-sanitize: | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(BBS_SOURCE) $(BBS_TEST) \
		-o $(BBS_SANITIZE_BINARY)
	./$(BBS_SANITIZE_BINARY)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(GRAM_SCHMIDT_SOURCE) $(GRAM_SCHMIDT_TEST) \
		-lm \
		-o $(GRAM_SCHMIDT_SANITIZE_BINARY)
	./$(GRAM_SCHMIDT_SANITIZE_BINARY)

clean:
	rm -rf $(BUILD_DIR)
