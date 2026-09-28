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

echo "All BioHash CLI integration tests passed."
