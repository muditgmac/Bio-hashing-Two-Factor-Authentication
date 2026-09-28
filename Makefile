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

DCT_SOURCE := src/dct.c
DCT_TEST := tests/test_dct.c
DCT_TEST_BINARY := $(BUILD_DIR)/test_dct
DCT_SANITIZE_BINARY := $(BUILD_DIR)/test_dct_sanitize

BIOHASH_SOURCE := src/biohash.c
BIOHASH_TEST := tests/test_biohash.c
BIOHASH_TEST_BINARY := $(BUILD_DIR)/test_biohash
BIOHASH_SANITIZE_BINARY := $(BUILD_DIR)/test_biohash_sanitize

MATCHER_SOURCE := src/matcher.c
MATCHER_TEST := tests/test_matcher.c
MATCHER_TEST_BINARY := $(BUILD_DIR)/test_matcher
MATCHER_SANITIZE_BINARY := $(BUILD_DIR)/test_matcher_sanitize

.PHONY: all test test-bbs test-gram-schmidt test-dct test-biohash test-matcher test-sanitize clean

all: test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BBS_TEST_BINARY): $(BBS_SOURCE) $(BBS_TEST) include/bbs.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(BBS_SOURCE) $(BBS_TEST) -o $(BBS_TEST_BINARY)

$(GRAM_SCHMIDT_TEST_BINARY): $(GRAM_SCHMIDT_SOURCE) $(GRAM_SCHMIDT_TEST) include/gram_schmidt.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(GRAM_SCHMIDT_SOURCE) $(GRAM_SCHMIDT_TEST) -lm -o $(GRAM_SCHMIDT_TEST_BINARY)

$(DCT_TEST_BINARY): $(DCT_SOURCE) $(DCT_TEST) include/dct.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DCT_SOURCE) $(DCT_TEST) -lm -o $(DCT_TEST_BINARY)

$(BIOHASH_TEST_BINARY): \
	$(BBS_SOURCE) \
	$(GRAM_SCHMIDT_SOURCE) \
	$(DCT_SOURCE) \
	$(BIOHASH_SOURCE) \
	$(BIOHASH_TEST) \
	include/bbs.h \
	include/gram_schmidt.h \
	include/dct.h \
	include/biohash.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		$(BBS_SOURCE) \
		$(GRAM_SCHMIDT_SOURCE) \
		$(DCT_SOURCE) \
		$(BIOHASH_SOURCE) \
		$(BIOHASH_TEST) \
		-lm \
		-o $(BIOHASH_TEST_BINARY)

test-bbs: $(BBS_TEST_BINARY)
	./$(BBS_TEST_BINARY)

test-gram-schmidt: $(GRAM_SCHMIDT_TEST_BINARY)
	./$(GRAM_SCHMIDT_TEST_BINARY)

test-dct: $(DCT_TEST_BINARY)
	./$(DCT_TEST_BINARY)

$(MATCHER_TEST_BINARY): $(MATCHER_SOURCE) $(MATCHER_TEST) include/matcher.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(MATCHER_SOURCE) $(MATCHER_TEST) -lm -o $(MATCHER_TEST_BINARY)

test-biohash: $(BIOHASH_TEST_BINARY)
	./$(BIOHASH_TEST_BINARY)

test-matcher: $(MATCHER_TEST_BINARY)
	./$(MATCHER_TEST_BINARY)

test: test-bbs test-gram-schmidt test-dct test-biohash test-matcher

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
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(DCT_SOURCE) $(DCT_TEST) \
		-lm \
		-o $(DCT_SANITIZE_BINARY)
	./$(DCT_SANITIZE_BINARY)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(BBS_SOURCE) \
		$(GRAM_SCHMIDT_SOURCE) \
		$(DCT_SOURCE) \
		$(BIOHASH_SOURCE) \
		$(BIOHASH_TEST) \
		-lm \
		-o $(BIOHASH_SANITIZE_BINARY)
	./$(BIOHASH_SANITIZE_BINARY)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(MATCHER_SOURCE) $(MATCHER_TEST) \
		-lm \
		-o $(MATCHER_SANITIZE_BINARY)
	./$(MATCHER_SANITIZE_BINARY)

clean:
	rm -rf $(BUILD_DIR)
