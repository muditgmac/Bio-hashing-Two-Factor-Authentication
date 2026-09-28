#!/usr/bin/env python3

import argparse
import csv
import hashlib
import json
from pathlib import Path

import numpy as np
import sklearn
from sklearn.datasets import fetch_olivetti_faces


def average_pool(
    images: np.ndarray,
    output_side: int,
) -> np.ndarray:
    if images.ndim != 3:
        raise ValueError("Expected images with shape samples x height x width.")

    sample_count, height, width = images.shape

    if height != width:
        raise ValueError("Expected square input images.")

    if output_side <= 0 or output_side > height:
        raise ValueError("Invalid pooled image size.")

    if height % output_side != 0:
        raise ValueError(
            "Input image side must be divisible by pooled image side."
        )

    block_size = height // output_side

    pooled = images.reshape(
        sample_count,
        output_side,
        block_size,
        output_side,
        block_size,
    ).mean(axis=(2, 4))

    return pooled


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()

    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)

    return digest.hexdigest()


def write_feature_csv(
    output_path: Path,
    labels: np.ndarray,
    feature_matrix: np.ndarray,
) -> None:
    output_path.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    feature_count = feature_matrix.shape[1]

    with output_path.open(
        "w",
        newline="",
        encoding="utf-8",
    ) as handle:
        writer = csv.writer(handle)

        writer.writerow(
            ["subject_id"]
            + [
                f"f{index}"
                for index in range(feature_count)
            ]
        )

        for label, features in zip(
            labels,
            feature_matrix,
            strict=True,
        ):
            writer.writerow(
                [int(label)]
                + [
                    format(float(value), ".9g")
                    for value in features
                ]
            )


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Prepare the scikit-learn Olivetti/AT&T face dataset "
            "for the BioHash verification CLI."
        )
    )

    parser.add_argument(
        "--output",
        type=Path,
        default=Path(
            "data/generated/olivetti_faces_16x16.csv"
        ),
        help="Output feature CSV.",
    )

    parser.add_argument(
        "--data-home",
        type=Path,
        default=Path("data/cache"),
        help="Local scikit-learn dataset cache.",
    )

    parser.add_argument(
        "--pooled-side",
        type=int,
        default=16,
        help="Width and height after average pooling.",
    )

    return parser.parse_args()


def main() -> None:
    args = parse_arguments()

    dataset = fetch_olivetti_faces(
        data_home=str(args.data_home),
        shuffle=False,
        download_if_missing=True,
    )

    images = np.asarray(
        dataset.images,
        dtype=np.float64,
    )

    labels = np.asarray(
        dataset.target,
        dtype=np.int64,
    )

    pooled = average_pool(
        images,
        args.pooled_side,
    )

    feature_matrix = pooled.reshape(
        pooled.shape[0],
        -1,
    )

    if not np.all(np.isfinite(feature_matrix)):
        raise RuntimeError(
            "Prepared feature matrix contains non-finite values."
        )

    write_feature_csv(
        args.output,
        labels,
        feature_matrix,
    )

    metadata_path = args.output.with_suffix(
        ".metadata.json"
    )

    metadata = {
        "dataset": "Olivetti faces / AT&T Database of Faces",
        "loader": "sklearn.datasets.fetch_olivetti_faces",
        "sklearn_version": sklearn.__version__,
        "sample_count": int(feature_matrix.shape[0]),
        "subject_count": int(np.unique(labels).size),
        "source_image_shape": [
            int(images.shape[1]),
            int(images.shape[2]),
        ],
        "pooled_image_shape": [
            int(args.pooled_side),
            int(args.pooled_side),
        ],
        "feature_count": int(feature_matrix.shape[1]),
        "preprocessing": (
            "non-overlapping block average pooling; "
            "row-major flattening"
        ),
        "label_minimum": int(labels.min()),
        "label_maximum": int(labels.max()),
        "feature_minimum": float(feature_matrix.min()),
        "feature_maximum": float(feature_matrix.max()),
        "csv_sha256": sha256_file(args.output),
        "attribution": "AT&T Laboratories Cambridge",
        "source_reference": (
            "https://cam-orl.co.uk/facedatabase.html"
        ),
    }

    metadata_path.write_text(
        json.dumps(
            metadata,
            indent=2,
            sort_keys=True,
        )
        + "\n",
        encoding="utf-8",
    )

    print(
        f"Prepared {feature_matrix.shape[0]} samples "
        f"from {np.unique(labels).size} subjects."
    )
    print(
        f"Feature dimensionality: "
        f"{feature_matrix.shape[1]}"
    )
    print(f"CSV:      {args.output}")
    print(f"Metadata: {metadata_path}")
    print(
        "SHA-256:  "
        f"{metadata['csv_sha256']}"
    )


if __name__ == "__main__":
    main()
