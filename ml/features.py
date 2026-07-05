"""
Feature extraction for sorting algorithm recommendation.

Extracts numerical features from an unsorted array that characterize
its structure and statistics, enabling a classifier to predict which
sorting algorithm will perform best.
"""

import math
from typing import List, Dict


def extract_features(data: List[int]) -> Dict[str, float]:
    """
    Extract interpretable feature dict from input data.

    Returns a dict mapping feature names to float values.
    Handles empty arrays gracefully (all zeros).
    """
    n = len(data)
    if n == 0:
        return {name: 0.0 for name in _feature_names()}

    features: Dict[str, float] = {}
    features["size"] = float(n)
    features["log_size"] = math.log2(n) if n > 1 else 0.0

    # -- Basic statistics --
    min_val = min(data)
    max_val = max(data)
    features["min"] = float(min_val)
    features["max"] = float(max_val)
    val_range = max_val - min_val
    features["range"] = float(val_range) if val_range > 0 else 1.0

    mean_val = sum(data) / n
    features["mean"] = float(mean_val)

    variance = sum((x - mean_val) ** 2 for x in data) / n
    std_val = math.sqrt(variance)
    features["std"] = float(std_val)

    # Coefficient of variation (robust to zero mean)
    features["cv"] = std_val / abs(mean_val) if abs(mean_val) > 1e-10 else 0.0

    # -- Structural features --

    # Ratio of unique elements
    unique_count = len(set(data))
    features["unique_ratio"] = unique_count / n

    # Constant array detection
    features["is_constant"] = 1.0 if unique_count == 1 else 0.0

    # Adjacent inversion ratio: fraction of adjacent pairs that are out of order
    inv_count = sum(1 for i in range(n - 1) if data[i] > data[i + 1])
    features["adj_inv_ratio"] = inv_count / (n - 1) if n > 1 else 0.0

    # Longest ascending prefix (high = already sorted from the start)
    asc_len = 1
    for i in range(1, n):
        if data[i - 1] <= data[i]:
            asc_len += 1
        else:
            break
    features["sorted_prefix_ratio"] = asc_len / n

    # Longest descending prefix (high = reversed from the start)
    desc_len = 1
    for i in range(1, n):
        if data[i - 1] >= data[i]:
            desc_len += 1
        else:
            break
    features["reversed_prefix_ratio"] = desc_len / n

    # Entropy: discretize into 10 equal-width bins
    if val_range > 0:
        bins = [0] * 10
        bin_width = val_range / 10.0
        for x in data:
            idx = min(9, int((x - min_val) / bin_width))
            bins[idx] += 1
        entropy = 0.0
        for c in bins:
            if c > 0:
                p = c / n
                entropy -= p * math.log2(p)
        features["entropy"] = entropy
    else:
        features["entropy"] = 0.0

    return features


def feature_vector(data: List[int]) -> List[float]:
    """Extract features as a flat list in feature_names() order."""
    feats = extract_features(data)
    return [feats[name] for name in _feature_names()]


def feature_names() -> List[str]:
    """Ordered list of feature names (matches feature_vector ordering)."""
    return _feature_names()


def _feature_names() -> List[str]:
    return [
        "size",
        "log_size",
        "min",
        "max",
        "range",
        "mean",
        "std",
        "cv",
        "unique_ratio",
        "is_constant",
        "adj_inv_ratio",
        "sorted_prefix_ratio",
        "reversed_prefix_ratio",
        "entropy",
    ]
