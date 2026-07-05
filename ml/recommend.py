"""
Recommend the best sorting algorithm for given input data.

Loads a pre-trained model (ml/model.joblib) and extracts features
to predict which sorting algorithm will perform best.

Usage:
    from ml.recommend import recommend

    result = recommend([5, 3, 1, 4, 2])
    print(result["algorithm"])    # "InsertionSort"
    print(result["confidence"])   # 0.92
"""

import os
import sys
from typing import List, Dict

# Ensure project root is on sys.path
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_PROJ_ROOT = os.path.dirname(_SCRIPT_DIR)
if _PROJ_ROOT not in sys.path:
    sys.path.insert(0, _PROJ_ROOT)

from ml.features import feature_vector


def _load_payload():
    """Lazy-load model payload from model.joblib (cached after first call)."""
    if not hasattr(_load_payload, "_cache"):
        import joblib
        model_path = os.path.join(_SCRIPT_DIR, "model.joblib")
        if not os.path.exists(model_path):
            raise FileNotFoundError(
                f"Model not found at {model_path}. "
                "Run `python ml/train_selector.py` first to train the model."
            )
        _load_payload._cache = joblib.load(model_path)
    return _load_payload._cache


def recommend(data: List[int]) -> Dict:
    """Recommend the best sorting algorithm for *data*.

    Args:
        data: List of integers to sort.

    Returns:
        Dict with keys:
          - "algorithm":  predicted best algorithm name
          - "confidence": probability of the prediction (0 ~ 1)
          - "probabilities": dict mapping algorithm -> probability
    """
    payload = _load_payload()
    model = payload["model"]
    scaler = payload["scaler"]

    feats = feature_vector(data)
    feat_array = [feats]

    feat_scaled = scaler.transform(feat_array)
    pred = model.predict(feat_scaled)[0]
    probs = model.predict_proba(feat_scaled)[0]

    confidence = float(max(probs))
    prob_dict = {
        str(algo): float(prob)
        for algo, prob in zip(model.classes_, probs)
    }

    return {
        "algorithm": str(pred),
        "confidence": round(confidence, 4),
        "probabilities": prob_dict,
    }
