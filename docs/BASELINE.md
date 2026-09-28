# Baseline Assessment

## Original implementation

The repository begins as a compact C implementation demonstrating a
BioHash-style pipeline composed of:

1. Blum-Blum-Shub-style binary sequence generation
2. Gram-Schmidt orthonormalisation
3. Discrete Cosine Transform
4. Random projection / inner-product computation
5. Threshold-based binary BioHash generation

The original implementation is intentionally preserved in Git history
before the modularisation and evaluation work.

## Baseline build

The original programs compile with Clang under C11 with AddressSanitizer
and UndefinedBehaviorSanitizer enabled.

Compiler diagnostics observed:

- Project.c: 12 warnings
- Blum_Prime_Generator.c: 4 warnings
- Discrete_Cosine_Transform.c: 1 warning
- Gram_Schmidt_Orthogonalisation.c: 1 warning

## Known baseline limitations

- Monolithic implementation with limited separation of concerns
- No automated tests
- No reproducible experiment configuration
- No biometric verification metrics
- No genuine/impostor score evaluation
- No revocability or unlinkability evaluation
- Pseudorandom parameters rely on rand()
- Several bounds, type-safety, and numerical robustness issues
- Random feature vectors are used instead of a real biometric dataset
- Minimal project documentation

## Modernisation objective

Refactor the prototype into a reproducible cancellable-biometric
evaluation platform with modular C components, automated tests,
deterministic experimental controls, Hamming-distance matching,
FAR/FRR/EER evaluation, revocability experiments, unlinkability
experiments, CI, and technical documentation.
