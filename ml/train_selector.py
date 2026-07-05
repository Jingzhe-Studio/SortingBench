"""
Train a classifier that recommends the best sorting algorithm for any input.

Generates synthetic training data from algorithmic complexity theory
(no .pyd required), or loads real benchmark CSV when available.

Outputs: ml/model.joblib  (model + scaler + feature names)
"""

import math
import random
import os
import sys
from typing import List, Tuple, Optional

import numpy as np
from sklearn.ensemble import RandomForestClassifier
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split, cross_val_score
from sklearn.metrics import accuracy_score, classification_report
from sklearn.dummy import DummyClassifier

# Ensure project root is on sys.path so "from ml.features" works as a script
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_PROJ_ROOT = os.path.dirname(_SCRIPT_DIR)
if _PROJ_ROOT not in sys.path:
    sys.path.insert(0, _PROJ_ROOT)

from ml.features import feature_vector, feature_names

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

ALGORITHMS = [
    "BubbleSort",
    "SelectionSort",
    "InsertionSort",
    "ShellSort",
    "MergeSort",
    "QuickSort",
]

DATA_TYPES = [
    "random",
    "sorted",
    "reversed",
    "nearly_sorted",
    "duplicates",
    "constant",
]

# Algorithmic constant factors (relative, lower = faster for same complexity)
_CONSTANTS = {
    "BubbleSort": 0.50,
    "SelectionSort": 0.40,
    "InsertionSort": 0.25,
    "ShellSort": 0.15,
    "MergeSort": 0.20,
    "QuickSort": 0.10,
}

# Small-array overhead bonus for simple algorithms (subtracted from effective time)
# Insertion sort and selection sort have very low overhead for small n
_SMALL_BONUS = {
    "BubbleSort": 0.0,
    "SelectionSort": 0.05,
    "InsertionSort": 0.10,
    "ShellSort": 0.0,
    "MergeSort": 0.0,
    "QuickSort": 0.0,
}


# ---------------------------------------------------------------------------
# Synthetic time estimation
# ---------------------------------------------------------------------------

def _complexity(algo: str, data_type: str, n: int) -> float:
    """Return complexity magnitude (abstract units)."""
    if n <= 1:
        return 1.0

    log_n = math.log2(n)
    n_log_n = n * log_n

    # MergeSort — always O(n log n)
    if algo == "MergeSort":
        return n_log_n

    # ShellSort — O(n^4/3) typical
    if algo == "ShellSort":
        return n ** 1.3

    # QuickSort — O(n log n) average, O(n²) worst
    if algo == "QuickSort":
        if data_type in ("sorted", "reversed", "constant"):
            return n * n  # naive pivot degrades
        if data_type == "nearly_sorted":
            return n ** 1.6
        return n_log_n

    # BubbleSort — O(n) best, O(n²) worst
    if algo == "BubbleSort":
        if data_type in ("sorted", "constant", "nearly_sorted"):
            return n
        return n * n

    # InsertionSort — O(n) best, O(n²) worst
    if algo == "InsertionSort":
        if data_type in ("sorted", "constant", "nearly_sorted"):
            return n
        return n * n

    # SelectionSort — always O(n²)
    if algo == "SelectionSort":
        return n * n

    return n * n


def _estimate_time(algo: str, data_type: str, n: int, rng: np.random.RandomState) -> float:
    """Simulate a realistic execution time in abstract units."""
    c = _CONSTANTS.get(algo, 1.0)
    bonus = _SMALL_BONUS.get(algo, 0.0)
    complexity = _complexity(algo, data_type, n)
    time_val = c * complexity * (1.0 - bonus * math.exp(-n / 50))
    # Log-normal noise (±~15%)
    noise = math.exp(rng.normal(0, 0.15))
    return time_val * noise


# ---------------------------------------------------------------------------
# Data generation helpers
# ---------------------------------------------------------------------------

def _make_array(data_type: str, n: int, rng: random.Random) -> List[int]:
    """Generate a list of ints matching the given data type."""
    if n == 0:
        return []
    if data_type == "random":
        return [rng.randint(0, 10000) for _ in range(n)]
    if data_type == "sorted":
        return list(range(n))
    if data_type == "reversed":
        return list(range(n, 0, -1))
    if data_type == "nearly_sorted":
        data = list(range(n))
        swaps = max(1, n // 20)
        for _ in range(swaps):
            i = rng.randint(0, n - 1)
            j = rng.randint(0, n - 1)
            data[i], data[j] = data[j], data[i]
        return data
    if data_type == "duplicates":
        return [rng.randint(0, max(1, n // 10)) for _ in range(n)]
    if data_type == "constant":
        return [42] * n
    # fallback
    return [rng.randint(0, 10000) for _ in range(n)]


# ---------------------------------------------------------------------------
# Training data generation
# ---------------------------------------------------------------------------

def _generate_synthetic_data(
    samples_per_type: int = 80,
    seed: int = 42,
) -> Tuple[np.ndarray, np.ndarray]:
    """Generate synthetic labeled training data.

    For each data type and a range of sizes, creates arrays and labels
    them with the theoretically fastest algorithm (plus noise).
    """
    py_rng = random.Random(seed)
    np_rng = np.random.RandomState(seed)

    # Continuous range of sizes (log-uniform)
    sizes = np.unique(np.logspace(
        1, 4.5, num=120, dtype=int
    )).tolist()  # 10 ~ 31622

    X_list: List[List[float]] = []
    y_list: List[str] = []

    for data_type in DATA_TYPES:
        for n in sizes:
            for _ in range(samples_per_type):
                data = _make_array(data_type, n, py_rng)

                # Estimate time for each algorithm
                times = {
                    algo: _estimate_time(algo, data_type, n, np_rng)
                    for algo in ALGORITHMS
                }
                best = min(times, key=times.get)

                feats = feature_vector(data)
                X_list.append(feats)
                y_list.append(best)

    return np.array(X_list, dtype=np.float64), np.array(y_list)


# ---------------------------------------------------------------------------
# Real CSV loading
# ---------------------------------------------------------------------------

def _load_csv_data(csv_path: str) -> Tuple[np.ndarray, np.ndarray]:
    """Load training data from a TrainingExport CSV.

    Expected columns: data, best_algorithm, (plus metadata columns).
    The 'data' column contains space-separated integers.
    """
    import pandas as pd

    df = pd.read_csv(csv_path)
    required = {"data", "best_algorithm"}
    missing = required - set(df.columns)
    if missing:
        raise ValueError(f"CSV missing columns: {missing}")

    X_list: List[List[float]] = []
    y_list: List[str] = []

    for _, row in df.iterrows():
        raw = str(row["data"])
        data = [int(x) for x in raw.split() if x.strip()]
        if len(data) == 0:
            continue
        X_list.append(feature_vector(data))
        y_list.append(str(row["best_algorithm"]))

    return np.array(X_list, dtype=np.float64), np.array(y_list)


# ---------------------------------------------------------------------------
# Training
# ---------------------------------------------------------------------------

def train(
    csv_path: Optional[str] = None,
    samples_per_type: int = 80,
) -> Tuple[RandomForestClassifier, StandardScaler, float]:
    """Train the algorithm selector.

    Args:
        csv_path: Optional path to real benchmark CSV.
                  If None, generates synthetic training data.
        samples_per_type: Number of samples per (type, size) combo.

    Returns:
        (model, scaler, test_accuracy)
    """
    if csv_path and os.path.isfile(csv_path):
        print(f"Loading benchmark data from {csv_path} ...")
        X, y = _load_csv_data(csv_path)
    else:
        print("Generating synthetic training data ...")
        X, y = _generate_synthetic_data(samples_per_type=samples_per_type)

    print(f"Samples: {len(X)}  |  Features: {X.shape[1]}  |  Classes: {len(set(y))}")

    # Split
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42, stratify=y,
    )

    # Scale
    scaler = StandardScaler()
    X_train_s = scaler.fit_transform(X_train)
    X_test_s = scaler.transform(X_test)

    # Train
    model = RandomForestClassifier(
        n_estimators=250,
        max_depth=15,
        min_samples_split=5,
        random_state=42,
        class_weight="balanced",
        n_jobs=-1,
    )
    model.fit(X_train_s, y_train)

    # Evaluate
    y_pred = model.predict(X_test_s)
    acc = accuracy_score(y_test, y_pred)
    print(f"\nTest accuracy: {acc:.4f}")
    print("\n--- Classification Report ---")
    print(classification_report(y_test, y_pred, zero_division=0))

    # --- Baselines ---
    # 1. Most-frequent class
    dummy_most = DummyClassifier(strategy="most_frequent", random_state=42)
    dummy_most.fit(X_train_s, y_train)
    bl_most = accuracy_score(y_test, dummy_most.predict(X_test_s))

    # 2. Always predict QuickSort
    dummy_qs = DummyClassifier(strategy="constant", constant="QuickSort", random_state=42)
    dummy_qs.fit(X_train_s, y_train)
    bl_qs = accuracy_score(y_test, dummy_qs.predict(X_test_s))

    print(f"\n--- Baseline Comparisons ---")
    print(f"Model (RandomForest):           {acc:.4f}")
    print(f"Baseline (most-frequent class): {bl_most:.4f}")
    print(f"Baseline (always QuickSort):    {bl_qs:.4f}")
    print(f"Improvement over QuickSort:     {acc - bl_qs:+.4f}")

    # Cross-validation
    cv = cross_val_score(model, X_train_s, y_train, cv=5)
    print(f"\nCV accuracy:  {cv.mean():.4f} +/- {cv.std() * 2:.4f}")

    # Feature importance
    importances = sorted(
        zip(feature_names(), model.feature_importances_),
        key=lambda x: x[1],
        reverse=True,
    )
    print("\n--- Top-5 Features ---")
    for name, imp in importances[:5]:
        print(f"  {name:25s}  {imp:.4f}")

    return model, scaler, acc


def save_model(model: RandomForestClassifier, scaler: StandardScaler, path: Optional[str] = None):
    """Persist model + scaler + feature names via joblib."""
    if path is None:
        path = os.path.join(_SCRIPT_DIR, "model.joblib")

    import joblib
    payload = {
        "model": model,
        "scaler": scaler,
        "feature_names": feature_names(),
        "classes": model.classes_.tolist(),
    }
    joblib.dump(payload, path)
    print(f"\nModel saved to {path}")


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Train sorting algorithm selector")
    parser.add_argument("--csv", help="Path to benchmark CSV (optional)")
    parser.add_argument("--samples", type=int, default=80, help="Samples per type/size")
    parser.add_argument("--output", help="Model output path (default: ml/model.joblib)")
    args = parser.parse_args()

    model, scaler, acc = train(csv_path=args.csv, samples_per_type=args.samples)
    save_model(model, scaler, path=args.output)
