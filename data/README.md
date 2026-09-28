# Biometric datasets

Real biometric data are not committed to this repository.

## Olivetti / AT&T Database of Faces

The real-data verification example uses the Olivetti faces dataset
distributed through `sklearn.datasets.fetch_olivetti_faces`.

The underlying Database of Faces originated at AT&T Laboratories
Cambridge. The original database contains 10 face images for each of
40 subjects.

The scikit-learn representation used by this project contains 400
64 x 64 grayscale images with pixel values represented on [0, 1].

For the BioHash evaluation workflow, the preparation script performs
deterministic non-overlapping average pooling from 64 x 64 to 16 x 16,
then flattens each image in row-major order to produce 256 numeric
features.

No random augmentation, train/test shuffling, or learned feature
extraction is performed during this preparation step.

Run:

    python3 tools/prepare_olivetti.py

Generated data are written under `data/generated/` and cached source
data under `data/cache/`. Both directories are intentionally excluded
from version control.

Dataset credit:

    AT&T Laboratories Cambridge

Original database information:

    https://cam-orl.co.uk/facedatabase.html
