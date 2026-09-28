# Cancellable BioHash Two-Factor Authentication

[![CI](https://github.com/muditgmac/Bio-hashing-Two-Factor-Authentication/actions/workflows/ci.yml/badge.svg)](https://github.com/muditgmac/Bio-hashing-Two-Factor-Authentication/actions/workflows/ci.yml)

A modular C implementation and experimental framework for
**cancellable biometric verification using BioHashing**.

The project combines a biometric feature representation with a
token-dependent transformation to generate binary protected templates.
It supports deterministic BioHash generation, Hamming-distance
matching, biometric verification metrics, template revocability
experiments, and quantitative cross-token unlinkability analysis.

The repository began as a compact academic BioHashing prototype and
has since been redesigned into a tested, modular evaluation framework.

---

## Overview

Traditional biometric systems face a fundamental problem: biometric
characteristics cannot simply be reissued if a stored template is
compromised.

Cancellable biometrics address this problem by applying a
token-dependent transformation before comparison or storage. A new
token can generate a different protected template without requiring
the underlying biometric characteristic itself to change.

This project implements and evaluates that concept using a
BioHash-style pipeline written in C.

At a high level:

    Biometric feature vector
            |
            v
    Orthonormal DCT-II
            |
            v
    Transformed feature vector
            |
            +-----------------------------+
                                          |
    Token configuration                   |
            |                             |
            v                             |
    Blum-Blum-Shub generator              |
            |                             |
            v                             |
    Pseudorandom projection matrix        |
            |                             |
            v                             |
    Modified Gram-Schmidt                 |
            |                             |
            v                             |
    Orthonormal token-dependent basis ----+
            |
            v
    Inner-product projections
            |
            v
    Threshold quantization
            |
            v
    Binary BioHash template
            |
            v
    Normalized Hamming-distance matching

---

## Core capabilities

The modernized implementation provides:

- deterministic Blum-Blum-Shub pseudorandom bit generation;
- token-dependent protected-template generation;
- orthonormal DCT-II feature transformation;
- Modified Gram-Schmidt orthonormalization;
- binary BioHash generation through projection and thresholding;
- Hamming and normalized Hamming-distance matching;
- threshold-based verification;
- FMR and FNMR calculation;
- equal-error-rate operating-point estimation;
- end-to-end genuine and impostor verification experiments;
- optional CSV export of experiment summaries and score distributions;
- template revocability / renewability analysis;
- cross-token mated and non-mated score generation;
- quantitative local and global unlinkability evaluation;
- unit and integration tests;
- AddressSanitizer and UndefinedBehaviorSanitizer validation.

---

## Repository structure

    .
    ├── include/
    │   ├── bbs.h
    │   ├── biohash.h
    │   ├── dct.h
    │   ├── evaluation.h
    │   ├── experiment.h
    │   ├── gram_schmidt.h
    │   ├── matcher.h
    │   ├── result_export.h
    │   ├── revocability.h
    │   ├── unlinkability.h
    │   └── unlinkability_metric.h
    │
    ├── src/
    │   ├── bbs.c
    │   ├── biohash.c
    │   ├── dct.c
    │   ├── evaluation.c
    │   ├── experiment.c
    │   ├── gram_schmidt.c
    │   ├── matcher.c
    │   ├── result_export.c
    │   ├── revocability.c
    │   ├── unlinkability.c
    │   └── unlinkability_metric.c
    │
    ├── tests/
    │   └── unit and integration tests
    │
    ├── docs/
    │   └── BASELINE.md
    │
    ├── Makefile
    └── legacy prototype source files

The original standalone C files are retained for historical comparison
with the modular implementation.

---

## BioHash generation

A protected binary template is generated from a feature vector and a
token-specific configuration.

The main stages are:

### 1. Feature transformation

The input feature vector is transformed using an orthonormal DCT-II.

### 2. Token-dependent pseudorandom generation

Blum-Blum-Shub parameters define a deterministic pseudorandom sequence.

Changing the token configuration changes the generated projection
basis.

### 3. Orthonormal basis construction

The generated matrix is converted into an orthonormal projection basis
using Modified Gram-Schmidt orthogonalization.

### 4. Projection

The transformed biometric features are projected onto the
token-dependent orthonormal basis.

### 5. Quantization

Each projection coefficient is compared against a configurable
threshold and converted to a binary value.

The result is a token-dependent binary BioHash template.

---

## Matching

Two BioHash templates are compared using Hamming distance.

For templates of length N:

    normalized distance = differing bit positions / N

The resulting value lies in the interval:

    0.0 <= distance <= 1.0

Lower values indicate greater similarity.

A verification decision accepts a comparison when:

    distance <= decision threshold

---

## Verification evaluation

The framework separates comparison scores into:

- **genuine comparisons** — samples belonging to the same identity;
- **impostor comparisons** — samples belonging to different identities.

It computes the following metrics.

### False Match Rate (FMR)

The fraction of impostor comparisons incorrectly accepted.

### False Non-Match Rate (FNMR)

The fraction of genuine comparisons incorrectly rejected.

### Equal Error Rate (EER)

An estimated operating point at which FMR and FNMR are closest.

The experiment runner can generate BioHashes for an entire feature
matrix, perform pairwise comparisons, and calculate verification
statistics.

---

## Revocability evaluation

A cancellable biometric system should permit a compromised protected
template to be replaced.

The revocability experiment generates multiple BioHashes from the same
feature vector using different token configurations and measures their
pairwise normalized Hamming distances.

Reported statistics include:

    minimum cross-token distance
    mean cross-token distance
    maximum cross-token distance
    number of identical template pairs

This evaluates protected-template diversity after token replacement.

Revocability alone does not establish unlinkability.

---

## Unlinkability evaluation

The framework evaluates whether protected templates generated under
different tokens can be linked to the same underlying identity.

Cross-token comparisons are separated into:

    mated cross-token scores
        same subject, different tokens

    non-mated cross-token scores
        different subjects, different tokens

The project supports both score-distribution generation and a
quantitative unlinkability metric.

The local metric is derived from the likelihood ratio between mated
and non-mated score distributions.

A global measure is then obtained by weighting the local measure by
the mated score distribution.

The resulting measure lies in:

    0 <= D_sys <= 1

where lower values indicate less evidence for cross-token linkability
under the evaluated empirical score model.

---

## Command-line verification

The repository provides a command-line program for running a complete
verification experiment from a CSV feature dataset.

Build the executable with:

    make cli

Run the bundled synthetic example with:

    ./build/biohash-evaluate \
        --input examples/demo_features.csv \
        --hash-length 3

The input CSV uses the following structure:

    subject_id,f1,f2,f3,...,fN
    1,0.12,0.45,0.18,...,0.81
    1,0.11,0.47,0.20,...,0.79
    2,-0.31,0.22,0.64,...,0.14

The first column identifies the subject. All remaining columns contain
finite numeric feature values, and every sample must have the same
feature dimensionality.

The CLI reports:

- dataset dimensions;
- BioHash/token configuration;
- genuine comparison count;
- impostor comparison count;
- mean genuine normalized Hamming distance;
- mean impostor normalized Hamming distance;
- estimated EER operating threshold;
- FMR and FNMR at the selected operating point;
- estimated EER.

Token parameters can also be supplied explicitly:

    ./build/biohash-evaluate \
        --input examples/demo_features.csv \
        --hash-length 3 \
        --p 499 \
        --q 547 \
        --seed 12345 \
        --threshold 0.0 \
        --tolerance 1e-12

### Exporting experiment results

Verification results can optionally be exported as CSV files by
supplying an output directory:

    ./build/biohash-evaluate \
        --input examples/demo_features.csv \
        --hash-length 3 \
        --output-dir evaluation/results/demo

The output directory is created when necessary. Three files are
written:

    evaluation/results/demo/
    ├── summary.csv
    ├── genuine_scores.csv
    └── impostor_scores.csv

`summary.csv` contains the experiment configuration, dataset
dimensions, comparison counts, mean normalized Hamming distances, and
the estimated EER operating point.

`genuine_scores.csv` contains the individual normalized Hamming
distances for same-identity comparisons.

`impostor_scores.csv` contains the individual normalized Hamming
distances for different-identity comparisons.

Generated files under `evaluation/results/` are ignored by Git so
experiment output does not need to be committed.

Display all supported options with:

    ./build/biohash-evaluate --help

The bundled example is synthetic and exists only to demonstrate the
software workflow. Its verification statistics are not biometric
benchmark results.

---

## Building

A C11-compatible compiler and the standard math library are required.

Build and execute the complete normal test suite with:

    make clean
    make test

The project uses strict compiler diagnostics including:

    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
    -Werror

---

## Sanitizer validation

AddressSanitizer and UndefinedBehaviorSanitizer builds are available
through:

    make clean
    make test-sanitize

These builds help detect memory-safety and undefined-behavior errors
during testing.

---

## Development methodology

The original implementation was preserved and assessed before
modernization.

Development then proceeded incrementally through independently tested
components:

    baseline assessment
            ↓
    Blum-Blum-Shub generator
            ↓
    Modified Gram-Schmidt
            ↓
    orthonormal DCT-II
            ↓
    integrated BioHash generation
            ↓
    Hamming-distance matcher
            ↓
    FMR / FNMR / EER evaluation
            ↓
    verification experiment runner
            ↓
    revocability evaluation
            ↓
    cross-token unlinkability scoring
            ↓
    quantitative unlinkability metric

Each stage was validated before integration into the larger pipeline.

---

## Current scope

The repository currently provides the computational and experimental
infrastructure required for cancellable BioHash evaluation.

The automated tests primarily validate the implementation using
controlled deterministic inputs.

A major next stage is benchmarking the framework on real biometric
feature data and reporting reproducible verification, revocability,
and unlinkability experiments.

---

## Security note

This repository is an experimental and educational implementation of
cancellable biometric techniques.

It has not undergone an independent security audit and should not be
treated as a production authentication system.

Cryptographic security, biometric performance, privacy protection,
and operational security require broader analysis before real-world
deployment.

---

## Historical implementation

The repository retains the original standalone implementation files:

    Project.c
    Blum_Prime_Generator.c
    Discrete_Cosine_Transform.c
    Gram_Schmidt_Orthogonalisation.c

These files represent the original prototype and are preserved to make
the modernization process transparent.

The modular implementation under `src/`, `include/`, and `tests/`
should be used for current development and evaluation.

---

## Planned work

Planned extensions include:

- real biometric dataset integration;
- reproducible experiment configuration;
- CSV result export;
- score-distribution visualization;
- verification performance plots;
- revocability experiments across subjects and tokens;
- larger-scale unlinkability analysis;
- continuous integration across supported compilers.

---

## Author

**Mudit Gupta**

GitHub: [@muditgmac](https://github.com/muditgmac)
