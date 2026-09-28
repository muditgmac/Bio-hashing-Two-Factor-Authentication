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

EVALUATION_SOURCE := src/evaluation.c
EVALUATION_TEST := tests/test_evaluation.c
EVALUATION_TEST_BINARY := $(BUILD_DIR)/test_evaluation
EVALUATION_SANITIZE_BINARY := $(BUILD_DIR)/test_evaluation_sanitize

ERROR_RATE_SWEEP_SOURCE := src/error_rate_sweep.c
ERROR_RATE_SWEEP_TEST := tests/test_error_rate_sweep.c
ERROR_RATE_SWEEP_TEST_BINARY := $(BUILD_DIR)/test_error_rate_sweep
ERROR_RATE_SWEEP_SANITIZE_BINARY := $(BUILD_DIR)/test_error_rate_sweep_sanitize

EXPERIMENT_SOURCE := src/experiment.c
EXPERIMENT_TEST := tests/test_experiment.c
EXPERIMENT_TEST_BINARY := $(BUILD_DIR)/test_experiment
EXPERIMENT_SANITIZE_BINARY := $(BUILD_DIR)/test_experiment_sanitize

REVOCABILITY_SOURCE := src/revocability.c
REVOCABILITY_TEST := tests/test_revocability.c
REVOCABILITY_TEST_BINARY := $(BUILD_DIR)/test_revocability
REVOCABILITY_SANITIZE_BINARY := $(BUILD_DIR)/test_revocability_sanitize

UNLINKABILITY_SOURCE := src/unlinkability.c
UNLINKABILITY_TEST := tests/test_unlinkability.c
UNLINKABILITY_TEST_BINARY := $(BUILD_DIR)/test_unlinkability
UNLINKABILITY_SANITIZE_BINARY := $(BUILD_DIR)/test_unlinkability_sanitize

UNLINKABILITY_METRIC_SOURCE := src/unlinkability_metric.c
UNLINKABILITY_METRIC_TEST := tests/test_unlinkability_metric.c
UNLINKABILITY_METRIC_TEST_BINARY := $(BUILD_DIR)/test_unlinkability_metric
UNLINKABILITY_METRIC_SANITIZE_BINARY := $(BUILD_DIR)/test_unlinkability_metric_sanitize

UNLINKABILITY_EXPERIMENT_SOURCE := src/unlinkability_experiment.c
UNLINKABILITY_EXPERIMENT_TEST := tests/test_unlinkability_experiment.c
UNLINKABILITY_EXPERIMENT_TEST_BINARY := $(BUILD_DIR)/test_unlinkability_experiment
UNLINKABILITY_EXPERIMENT_SANITIZE_BINARY := $(BUILD_DIR)/test_unlinkability_experiment_sanitize

DATASET_SOURCE := src/dataset.c
DATASET_TEST := tests/test_dataset.c
DATASET_TEST_BINARY := $(BUILD_DIR)/test_dataset
DATASET_SANITIZE_BINARY := $(BUILD_DIR)/test_dataset_sanitize

PREPROCESSING_SOURCE := src/preprocessing.c
PREPROCESSING_TEST := tests/test_preprocessing.c
PREPROCESSING_TEST_BINARY := $(BUILD_DIR)/test_preprocessing
PREPROCESSING_SANITIZE_BINARY := $(BUILD_DIR)/test_preprocessing_sanitize

CLI_SOURCE := src/biohash_evaluate.c
CLI_BINARY := $(BUILD_DIR)/biohash-evaluate
CLI_SANITIZE_BINARY := $(BUILD_DIR)/biohash-evaluate-sanitize
CLI_TEST := tests/test_cli.sh

RESULT_EXPORT_SOURCE := src/result_export.c
RESULT_EXPORT_TEST := tests/test_result_export.c
RESULT_EXPORT_TEST_BINARY := $(BUILD_DIR)/test_result_export
RESULT_EXPORT_SANITIZE_BINARY := $(BUILD_DIR)/test_result_export_sanitize

.PHONY: all test test-bbs test-gram-schmidt test-dct test-biohash test-matcher test-evaluation test-error-rate-sweep test-experiment test-revocability test-unlinkability test-unlinkability-metric test-unlinkability-experiment test-dataset test-preprocessing test-cli cli test-sanitize clean test-result-export

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

$(EVALUATION_TEST_BINARY): $(EVALUATION_SOURCE) $(EVALUATION_TEST) include/evaluation.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(EVALUATION_SOURCE) $(EVALUATION_TEST) -lm -o $(EVALUATION_TEST_BINARY)

test-evaluation: $(EVALUATION_TEST_BINARY)
	./$(EVALUATION_TEST_BINARY)

$(ERROR_RATE_SWEEP_TEST_BINARY): \
	$(EVALUATION_SOURCE) \
	$(ERROR_RATE_SWEEP_SOURCE) \
	$(ERROR_RATE_SWEEP_TEST) \
	include/evaluation.h \
	include/error_rate_sweep.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		$(EVALUATION_SOURCE) \
		$(ERROR_RATE_SWEEP_SOURCE) \
		$(ERROR_RATE_SWEEP_TEST) \
		-lm \
		-o $(ERROR_RATE_SWEEP_TEST_BINARY)

test-error-rate-sweep: $(ERROR_RATE_SWEEP_TEST_BINARY)
	./$(ERROR_RATE_SWEEP_TEST_BINARY)

$(EXPERIMENT_TEST_BINARY): \
	$(BBS_SOURCE) \
	$(GRAM_SCHMIDT_SOURCE) \
	$(DCT_SOURCE) \
	$(BIOHASH_SOURCE) \
	$(MATCHER_SOURCE) \
	$(EVALUATION_SOURCE) \
	$(EXPERIMENT_SOURCE) \
	$(EXPERIMENT_TEST) \
	include/bbs.h \
	include/gram_schmidt.h \
	include/dct.h \
	include/biohash.h \
	include/matcher.h \
	include/evaluation.h \
	include/experiment.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		$(BBS_SOURCE) \
		$(GRAM_SCHMIDT_SOURCE) \
		$(DCT_SOURCE) \
		$(BIOHASH_SOURCE) \
		$(MATCHER_SOURCE) \
		$(EVALUATION_SOURCE) \
		$(EXPERIMENT_SOURCE) \
		$(EXPERIMENT_TEST) \
		-lm \
		-o $(EXPERIMENT_TEST_BINARY)

test-experiment: $(EXPERIMENT_TEST_BINARY)
	./$(EXPERIMENT_TEST_BINARY)

$(REVOCABILITY_TEST_BINARY): \
	$(BBS_SOURCE) \
	$(GRAM_SCHMIDT_SOURCE) \
	$(DCT_SOURCE) \
	$(BIOHASH_SOURCE) \
	$(MATCHER_SOURCE) \
	$(REVOCABILITY_SOURCE) \
	$(REVOCABILITY_TEST) \
	include/bbs.h \
	include/gram_schmidt.h \
	include/dct.h \
	include/biohash.h \
	include/matcher.h \
	include/revocability.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		$(BBS_SOURCE) \
		$(GRAM_SCHMIDT_SOURCE) \
		$(DCT_SOURCE) \
		$(BIOHASH_SOURCE) \
		$(MATCHER_SOURCE) \
		$(REVOCABILITY_SOURCE) \
		$(REVOCABILITY_TEST) \
		-lm \
		-o $(REVOCABILITY_TEST_BINARY)

test-revocability: $(REVOCABILITY_TEST_BINARY)
	./$(REVOCABILITY_TEST_BINARY)

$(UNLINKABILITY_TEST_BINARY): \
	$(BBS_SOURCE) \
	$(GRAM_SCHMIDT_SOURCE) \
	$(DCT_SOURCE) \
	$(BIOHASH_SOURCE) \
	$(MATCHER_SOURCE) \
	$(UNLINKABILITY_SOURCE) \
	$(UNLINKABILITY_TEST) \
	include/bbs.h \
	include/gram_schmidt.h \
	include/dct.h \
	include/biohash.h \
	include/matcher.h \
	include/unlinkability.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		$(BBS_SOURCE) \
		$(GRAM_SCHMIDT_SOURCE) \
		$(DCT_SOURCE) \
		$(BIOHASH_SOURCE) \
		$(MATCHER_SOURCE) \
		$(UNLINKABILITY_SOURCE) \
		$(UNLINKABILITY_TEST) \
		-lm \
		-o $(UNLINKABILITY_TEST_BINARY)

test-unlinkability: $(UNLINKABILITY_TEST_BINARY)
	./$(UNLINKABILITY_TEST_BINARY)

$(UNLINKABILITY_METRIC_TEST_BINARY): \
	$(UNLINKABILITY_METRIC_SOURCE) \
	$(UNLINKABILITY_METRIC_TEST) \
	include/unlinkability_metric.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		$(UNLINKABILITY_METRIC_SOURCE) \
		$(UNLINKABILITY_METRIC_TEST) \
		-lm \
		-o $(UNLINKABILITY_METRIC_TEST_BINARY)

test-unlinkability-metric: $(UNLINKABILITY_METRIC_TEST_BINARY)
	./$(UNLINKABILITY_METRIC_TEST_BINARY)

$(UNLINKABILITY_EXPERIMENT_TEST_BINARY): \
	$(BBS_SOURCE) \
	$(GRAM_SCHMIDT_SOURCE) \
	$(DCT_SOURCE) \
	$(BIOHASH_SOURCE) \
	$(MATCHER_SOURCE) \
	$(UNLINKABILITY_SOURCE) \
	$(UNLINKABILITY_METRIC_SOURCE) \
	$(UNLINKABILITY_EXPERIMENT_SOURCE) \
	$(UNLINKABILITY_EXPERIMENT_TEST) \
	include/bbs.h \
	include/gram_schmidt.h \
	include/dct.h \
	include/biohash.h \
	include/matcher.h \
	include/unlinkability.h \
	include/unlinkability_metric.h \
	include/unlinkability_experiment.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		$(BBS_SOURCE) \
		$(GRAM_SCHMIDT_SOURCE) \
		$(DCT_SOURCE) \
		$(BIOHASH_SOURCE) \
		$(MATCHER_SOURCE) \
		$(UNLINKABILITY_SOURCE) \
		$(UNLINKABILITY_METRIC_SOURCE) \
		$(UNLINKABILITY_EXPERIMENT_SOURCE) \
		$(UNLINKABILITY_EXPERIMENT_TEST) \
		-lm \
		-o $(UNLINKABILITY_EXPERIMENT_TEST_BINARY)

test-unlinkability-experiment: $(UNLINKABILITY_EXPERIMENT_TEST_BINARY)
	./$(UNLINKABILITY_EXPERIMENT_TEST_BINARY)

$(DATASET_TEST_BINARY): \
	$(DATASET_SOURCE) \
	$(DATASET_TEST) \
	include/dataset.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		$(DATASET_SOURCE) \
		$(DATASET_TEST) \
		-lm \
		-o $(DATASET_TEST_BINARY)

test-dataset: $(DATASET_TEST_BINARY)
	./$(DATASET_TEST_BINARY)

$(PREPROCESSING_TEST_BINARY): \
	$(PREPROCESSING_SOURCE) \
	$(PREPROCESSING_TEST) \
	include/preprocessing.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		$(PREPROCESSING_SOURCE) \
		$(PREPROCESSING_TEST) \
		-lm \
		-o $(PREPROCESSING_TEST_BINARY)

test-preprocessing: $(PREPROCESSING_TEST_BINARY)
	./$(PREPROCESSING_TEST_BINARY)

$(CLI_BINARY): \
	$(BBS_SOURCE) \
	$(GRAM_SCHMIDT_SOURCE) \
	$(DCT_SOURCE) \
	$(BIOHASH_SOURCE) \
	$(MATCHER_SOURCE) \
	$(EVALUATION_SOURCE) \
	$(ERROR_RATE_SWEEP_SOURCE) \
	$(EXPERIMENT_SOURCE) \
	$(DATASET_SOURCE) \
	$(RESULT_EXPORT_SOURCE) \
	$(CLI_SOURCE) \
	include/bbs.h \
	include/gram_schmidt.h \
	include/dct.h \
	include/biohash.h \
	include/matcher.h \
	include/evaluation.h \
	include/error_rate_sweep.h \
	include/experiment.h \
	include/dataset.h \
	include/result_export.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		$(BBS_SOURCE) \
		$(GRAM_SCHMIDT_SOURCE) \
		$(DCT_SOURCE) \
		$(BIOHASH_SOURCE) \
		$(MATCHER_SOURCE) \
		$(EVALUATION_SOURCE) \
		$(ERROR_RATE_SWEEP_SOURCE) \
		$(EXPERIMENT_SOURCE) \
		$(DATASET_SOURCE) \
		$(RESULT_EXPORT_SOURCE) \
		$(CLI_SOURCE) \
		-lm \
		-o $(CLI_BINARY)

cli: $(CLI_BINARY)

test-cli: $(CLI_BINARY) $(CLI_TEST) examples/demo_features.csv
	sh $(CLI_TEST) $(CLI_BINARY)

test: test-bbs test-gram-schmidt test-dct test-biohash test-matcher test-evaluation test-error-rate-sweep test-experiment test-revocability test-unlinkability test-unlinkability-metric test-unlinkability-experiment test-dataset test-preprocessing test-cli test-result-export

$(RESULT_EXPORT_TEST_BINARY): \
	$(RESULT_EXPORT_SOURCE) \
	$(RESULT_EXPORT_TEST) \
	include/result_export.h \
	include/error_rate_sweep.h \
	include/experiment.h \
	include/evaluation.h \
	include/biohash.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) \
		$(RESULT_EXPORT_SOURCE) \
		$(RESULT_EXPORT_TEST) \
		-o $(RESULT_EXPORT_TEST_BINARY)

test-result-export: $(RESULT_EXPORT_TEST_BINARY)
	./$(RESULT_EXPORT_TEST_BINARY)

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
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(EVALUATION_SOURCE) $(EVALUATION_TEST) \
		-lm \
		-o $(EVALUATION_SANITIZE_BINARY)
	./$(EVALUATION_SANITIZE_BINARY)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(EVALUATION_SOURCE) \
		$(ERROR_RATE_SWEEP_SOURCE) \
		$(ERROR_RATE_SWEEP_TEST) \
		-lm \
		-o $(ERROR_RATE_SWEEP_SANITIZE_BINARY)
	./$(ERROR_RATE_SWEEP_SANITIZE_BINARY)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(BBS_SOURCE) \
		$(GRAM_SCHMIDT_SOURCE) \
		$(DCT_SOURCE) \
		$(BIOHASH_SOURCE) \
		$(MATCHER_SOURCE) \
		$(EVALUATION_SOURCE) \
		$(EXPERIMENT_SOURCE) \
		$(EXPERIMENT_TEST) \
		-lm \
		-o $(EXPERIMENT_SANITIZE_BINARY)
	./$(EXPERIMENT_SANITIZE_BINARY)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(BBS_SOURCE) \
		$(GRAM_SCHMIDT_SOURCE) \
		$(DCT_SOURCE) \
		$(BIOHASH_SOURCE) \
		$(MATCHER_SOURCE) \
		$(REVOCABILITY_SOURCE) \
		$(REVOCABILITY_TEST) \
		-lm \
		-o $(REVOCABILITY_SANITIZE_BINARY)
	./$(REVOCABILITY_SANITIZE_BINARY)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(BBS_SOURCE) \
		$(GRAM_SCHMIDT_SOURCE) \
		$(DCT_SOURCE) \
		$(BIOHASH_SOURCE) \
		$(MATCHER_SOURCE) \
		$(UNLINKABILITY_SOURCE) \
		$(UNLINKABILITY_TEST) \
		-lm \
		-o $(UNLINKABILITY_SANITIZE_BINARY)
	./$(UNLINKABILITY_SANITIZE_BINARY)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(UNLINKABILITY_METRIC_SOURCE) \
		$(UNLINKABILITY_METRIC_TEST) \
		-lm \
		-o $(UNLINKABILITY_METRIC_SANITIZE_BINARY)
	./$(UNLINKABILITY_METRIC_SANITIZE_BINARY)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(BBS_SOURCE) \
		$(GRAM_SCHMIDT_SOURCE) \
		$(DCT_SOURCE) \
		$(BIOHASH_SOURCE) \
		$(MATCHER_SOURCE) \
		$(UNLINKABILITY_SOURCE) \
		$(UNLINKABILITY_METRIC_SOURCE) \
		$(UNLINKABILITY_EXPERIMENT_SOURCE) \
		$(UNLINKABILITY_EXPERIMENT_TEST) \
		-lm \
		-o $(UNLINKABILITY_EXPERIMENT_SANITIZE_BINARY)
	./$(UNLINKABILITY_EXPERIMENT_SANITIZE_BINARY)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(DATASET_SOURCE) \
		$(DATASET_TEST) \
		-lm \
		-o $(DATASET_SANITIZE_BINARY)
	./$(DATASET_SANITIZE_BINARY)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(PREPROCESSING_SOURCE) \
		$(PREPROCESSING_TEST) \
		-lm \
		-o $(PREPROCESSING_SANITIZE_BINARY)
	./$(PREPROCESSING_SANITIZE_BINARY)
	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(BBS_SOURCE) \
		$(GRAM_SCHMIDT_SOURCE) \
		$(DCT_SOURCE) \
		$(BIOHASH_SOURCE) \
		$(MATCHER_SOURCE) \
		$(EVALUATION_SOURCE) \
		$(ERROR_RATE_SWEEP_SOURCE) \
		$(EXPERIMENT_SOURCE) \
		$(DATASET_SOURCE) \
		$(RESULT_EXPORT_SOURCE) \
		$(CLI_SOURCE) \
		-lm \
		-o $(CLI_SANITIZE_BINARY)
	./$(CLI_SANITIZE_BINARY) \
		--input examples/demo_features.csv \
		--hash-length 3 \
		--output-dir build/sanitize-cli-export \
		> /dev/null

	$(CC) $(CFLAGS) $(SANITIZERS) -g \
		$(RESULT_EXPORT_SOURCE) \
		$(RESULT_EXPORT_TEST) \
		-o $(RESULT_EXPORT_SANITIZE_BINARY)
	./$(RESULT_EXPORT_SANITIZE_BINARY)

clean:
	rm -rf $(BUILD_DIR)
