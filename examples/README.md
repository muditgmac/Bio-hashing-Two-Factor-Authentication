# Example dataset

`demo_features.csv` is a small synthetic dataset used to demonstrate the
command-line verification workflow.

It contains four samples, two subjects, and four numeric features per
sample.

The file is intentionally simple so that users can verify that the complete
CSV-to-BioHash experiment pipeline is functioning.

It is **not a real biometric benchmark** and its verification results must
not be interpreted as biometric performance claims.

Run the example with:

    make cli

    ./build/biohash-evaluate \
        --input examples/demo_features.csv \
        --hash-length 3

CSV format:

    subject_id,f1,f2,...,fN
    1,0.12,0.45,...,0.81
    1,0.11,0.47,...,0.79
    2,-0.31,0.22,...,0.14

All data rows must contain the same number of finite numeric features.
