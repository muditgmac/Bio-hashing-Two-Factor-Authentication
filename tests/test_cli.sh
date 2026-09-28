#!/bin/sh

set -eu

CLI="${1:-build/biohash-evaluate}"

if [ ! -x "$CLI" ]; then
    echo "CLI executable not found: $CLI" >&2
    exit 1
fi

TMP_DIR="$(mktemp -d)"

cleanup()
{
    rm -rf "$TMP_DIR"
}

trap cleanup EXIT HUP INT TERM

# ------------------------------------------------------------
# Help output
# ------------------------------------------------------------

"$CLI" --help > "$TMP_DIR/help.txt"

grep -Fq "Usage:" "$TMP_DIR/help.txt"
grep -Fq -- "--input FILE" "$TMP_DIR/help.txt"
grep -Fq -- "--hash-length N" "$TMP_DIR/help.txt"

# ------------------------------------------------------------
# Successful end-to-end verification experiment
# ------------------------------------------------------------

"$CLI" \
    --input examples/demo_features.csv \
    --hash-length 3 \
    > "$TMP_DIR/result.txt"

grep -Fq "BioHash Verification Experiment" \
    "$TMP_DIR/result.txt"

grep -Fq "Samples:                    4" \
    "$TMP_DIR/result.txt"

grep -Fq "Features per sample:        4" \
    "$TMP_DIR/result.txt"

grep -Fq "Hash length:                3" \
    "$TMP_DIR/result.txt"

grep -Fq "Genuine comparisons:        2" \
    "$TMP_DIR/result.txt"

grep -Fq "Impostor comparisons:       4" \
    "$TMP_DIR/result.txt"

grep -Fq "Estimated EER:              0.000000" \
    "$TMP_DIR/result.txt"

# ------------------------------------------------------------
# Missing input file must fail cleanly
# ------------------------------------------------------------

if "$CLI" \
    --input tests/data/does_not_exist.csv \
    --hash-length 3 \
    > "$TMP_DIR/missing-file.out" \
    2> "$TMP_DIR/missing-file.err"
then
    echo "Expected missing-file invocation to fail." >&2
    exit 1
fi

grep -Fq "I/O error" \
    "$TMP_DIR/missing-file.err"

# ------------------------------------------------------------
# Hash length exceeding feature dimensionality must fail
# ------------------------------------------------------------

if "$CLI" \
    --input examples/demo_features.csv \
    --hash-length 99 \
    > "$TMP_DIR/hash-length.out" \
    2> "$TMP_DIR/hash-length.err"
then
    echo "Expected oversized hash-length invocation to fail." >&2
    exit 1
fi

grep -Fq "exceeds feature count" \
    "$TMP_DIR/hash-length.err"

# ------------------------------------------------------------
# Missing required --hash-length must fail
# ------------------------------------------------------------

if "$CLI" \
    --input examples/demo_features.csv \
    > "$TMP_DIR/missing-argument.out" \
    2> "$TMP_DIR/missing-argument.err"
then
    echo "Expected missing hash-length invocation to fail." >&2
    exit 1
fi

grep -Fq -- "--hash-length is required" \
    "$TMP_DIR/missing-argument.err"

# ------------------------------------------------------------
# Unknown option must fail
# ------------------------------------------------------------

if "$CLI" \
    --input examples/demo_features.csv \
    --hash-length 3 \
    --unknown-option \
    > "$TMP_DIR/unknown.out" \
    2> "$TMP_DIR/unknown.err"
then
    echo "Expected unknown-option invocation to fail." >&2
    exit 1
fi

grep -Fq "unknown argument" \
    "$TMP_DIR/unknown.err"

# ------------------------------------------------------------
# Result export must create nested output directories and CSVs
# ------------------------------------------------------------

EXPORT_DIR="$TMP_DIR/export/nested/results"

"$CLI" \
    --input examples/demo_features.csv \
    --hash-length 3 \
    --output-dir "$EXPORT_DIR" \
    > "$TMP_DIR/export.out" \
    2> "$TMP_DIR/export.err"

test -d "$EXPORT_DIR"

test -f "$EXPORT_DIR/summary.csv"
test -f "$EXPORT_DIR/genuine_scores.csv"
test -f "$EXPORT_DIR/impostor_scores.csv"
test -f "$EXPORT_DIR/error_rates.csv"

grep -Fq "Results exported to:" \
    "$TMP_DIR/export.out"


grep -Fq "error rates:" \
    "$TMP_DIR/export.out"

grep -Fq \
    "input_file,sample_count,feature_count,hash_length" \
    "$EXPORT_DIR/summary.csv"

grep -Fq \
    '"examples/demo_features.csv",4,4,3,499,547,12345' \
    "$EXPORT_DIR/summary.csv"

head -n 1 "$EXPORT_DIR/genuine_scores.csv" |
    grep -Fqx "comparison_index,distance"

head -n 1 "$EXPORT_DIR/impostor_scores.csv" |
    grep -Fqx "comparison_index,distance"


head -n 1 "$EXPORT_DIR/error_rates.csv" |
    grep -Fqx "threshold,fmr,fnmr"

GENUINE_LINES=$(
    wc -l < "$EXPORT_DIR/genuine_scores.csv" |
    tr -d '[:space:]'
)

IMPOSTOR_LINES=$(
    wc -l < "$EXPORT_DIR/impostor_scores.csv" |
    tr -d '[:space:]'
)

SUMMARY_LINES=$(
    wc -l < "$EXPORT_DIR/summary.csv" |
    tr -d '[:space:]'
)


ERROR_RATE_LINES=$(
    wc -l < "$EXPORT_DIR/error_rates.csv" |
    tr -d '[:space:]'
)

test "$GENUINE_LINES" -eq 3
test "$IMPOSTOR_LINES" -eq 5
test "$SUMMARY_LINES" -eq 2
test "$ERROR_RATE_LINES" -eq 5

grep -Fqx "0,0" \
    "$EXPORT_DIR/genuine_scores.csv"

grep -Fqx "1,0" \
    "$EXPORT_DIR/genuine_scores.csv"

grep -Fqx "0,1" \
    "$EXPORT_DIR/impostor_scores.csv"

grep -Fqx "3,1" \
    "$EXPORT_DIR/impostor_scores.csv"


grep -Fqx "0,0,0" \
    "$EXPORT_DIR/error_rates.csv"

grep -Fqx "1,1,0" \
    "$EXPORT_DIR/error_rates.csv"

# ------------------------------------------------------------
# --output-dir without a value must fail
# ------------------------------------------------------------

if "$CLI" \
    --input examples/demo_features.csv \
    --hash-length 3 \
    --output-dir \
    > "$TMP_DIR/missing-output-dir.out" \
    2> "$TMP_DIR/missing-output-dir.err"
then
    echo "Expected missing --output-dir value to fail." >&2
    exit 1
fi

grep -Fq \
    -- "--output-dir requires a directory path" \
    "$TMP_DIR/missing-output-dir.err"

# ------------------------------------------------------------
# A file cannot be used as an output directory
# ------------------------------------------------------------

touch "$TMP_DIR/not-a-directory"

if "$CLI" \
    --input examples/demo_features.csv \
    --hash-length 3 \
    --output-dir "$TMP_DIR/not-a-directory/child" \
    > "$TMP_DIR/bad-output-dir.out" \
    2> "$TMP_DIR/bad-output-dir.err"
then
    echo "Expected invalid output directory to fail." >&2
    exit 1
fi

grep -Fq \
    "unable to create output directory" \
    "$TMP_DIR/bad-output-dir.err"

echo "All BioHash CLI integration tests passed."
