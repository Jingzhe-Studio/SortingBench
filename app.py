"""
app.py — Sorting Algorithm Benchmark Streamlit UI
===================================================
Member B deliverable.  Single-file Streamlit application.

Features
--------
1. Sidebar: algorithm multi-select, data type, data size, repeats, timeout
2. Run → BenchmarkRunner.run() → st.dataframe
3. Execution time bar chart (plotly)
4. Key operations bar chart (plotly)
5. CSV export (st.download_button)
6. ML recommendation block (calls ml/recommend.py)

Usage
-----
    streamlit run app.py
"""

from __future__ import annotations

import time
from typing import get_args

import pandas as pd
import plotly.express as px
import streamlit as st

# ============================================================
# Import C++ extension  (Member A)
# ============================================================
try:
    import sorting_bench as sb

    SB_AVAILABLE = True
except ImportError:
    SB_AVAILABLE = False

# ============================================================
# Page configuration
# ============================================================
st.set_page_config(
    page_title="Sorting Algorithm Benchmark",
    page_icon="🔢",
    layout="wide",
)

st.title("🔢 Sorting Algorithm Benchmark")
st.caption(
    "Compare 6 sorting algorithms — runtime, key operations, "
    "and AI-powered smart recommendations."
)

if not SB_AVAILABLE:
    st.error(
        "⚠️ **`sorting_bench` module not found.**  "
        "Ask Member A to build the C++ extension and place the `.pyd` "
        "file in the project root, then restart."
    )
    st.stop()

# ============================================================
# Constants
# ============================================================
ALL_ALGORITHMS: list[str] = [
    "BubbleSort",
    "SelectionSort",
    "InsertionSort",
    "ShellSort",
    "MergeSort",
    "QuickSort",
]

# Mapping: display name → C++ class
ALGO_CLASS_MAP: dict[str, type] = {
    "BubbleSort": sb.BubbleSort,
    "SelectionSort": sb.SelectionSort,
    "InsertionSort": sb.InsertionSort,
    "ShellSort": sb.ShellSort,
    "MergeSort": sb.MergeSort,
    "QuickSort": sb.QuickSort,
}

# Mapping: display label → DataType enum
DATA_TYPE_MAP: dict[str, tuple[str, "sb.DataType"]] = {
    "🎲 Random": ("random", sb.RANDOM),
    "🔽 Reversed": ("reversed", sb.REVERSED),
    "📋 Many Duplicates": ("duplicates", sb.DUPLICATES),
    "🟰 Constant (all equal)": ("constant", sb.CONSTANT),
    "✅ Sorted": ("sorted", sb.SORTED),
    "↗️ Nearly Sorted": ("nearly_sorted", sb.NEARLY_SORTED),
}

# ============================================================
# Sidebar — Control Panel
# ============================================================
with st.sidebar:
    st.header("⚙️ Configuration")

    # 1. Algorithm multi-select
    selected_algos: list[str] = st.multiselect(
        "Select algorithms to benchmark",
        options=ALL_ALGORITHMS,
        default=ALL_ALGORITHMS,
        help="Choose one or more sorting algorithms.",
    )

    # 2. Data type
    data_type_label: str = st.selectbox(
        "Data distribution",
        options=list(DATA_TYPE_MAP.keys()),
        index=0,
        help="The distribution of values in the generated array.",
    )
    data_type_key, data_type_enum = DATA_TYPE_MAP[data_type_label]

    # 3. Data size
    data_size: int = st.slider(
        "Data size (number of elements)",
        min_value=50,
        max_value=10_000,
        value=1_000,
        step=50,
        help="Larger sizes → longer runs → clearer algorithmic differences.",
    )

    # 4. Repeat count  (mapped to BenchmarkConfig.repeat_times)
    repeat: int = st.number_input(
        "Repeat (averaging runs)",
        min_value=1,
        max_value=30,
        value=3,
        step=1,
        help="Each algorithm runs this many times; results are averaged.",
    )

    # 5. Timeout  (mapped to BenchmarkConfig.timeout_ms)
    timeout_sec: int = st.number_input(
        "Timeout per algorithm (seconds)",
        min_value=1,
        max_value=300,
        value=30,
        step=1,
        help="An algorithm exceeding this limit is marked as timed out.",
    )

    st.divider()

    # ▶ Run button
    run_btn: bool = st.button(
        "▶️  Run Benchmark", type="primary", use_container_width=True
    )

# ============================================================
# Data generation  (uses C++ DataGenerator for consistency)
# ============================================================
def generate_data(dtype: "sb.DataType", size: int) -> list[int]:
    """Generate a Python list for the requested data type and size."""
    return sb.DataGenerator.generate_by_type(dtype, size)


# ============================================================
# Run logic
# ============================================================
if run_btn:
    if not selected_algos:
        st.warning("⚠️ Please select at least one algorithm.")
    else:
        with st.spinner("Running benchmark…"):
            # --- Generate data -------------------------------------------------
            data: list[int] = generate_data(data_type_enum, data_size)
            st.session_state["last_data"] = data
            st.session_state["last_data_type"] = data_type_key
            st.session_state["last_data_size"] = data_size
            st.session_state["last_repeat"] = repeat

            # --- Build config --------------------------------------------------
            config = sb.BenchmarkConfig()
            config.data_type = data_type_key      # string: 'random', 'reversed', ...
            config.repeat_times = repeat
            config.timeout_ms = timeout_sec * 1000

            # --- Build runner, register sorters --------------------------------
            runner = sb.BenchmarkRunner()
            for algo in selected_algos:
                runner.add_sorter(ALGO_CLASS_MAP[algo]())

            # --- Run  (C++ layer handles repeats internally) -------------------
            raw_results = runner.run(data, config)

            # --- Convert to DataFrame ------------------------------------------
            rows: list[dict] = []
            for r in raw_results:
                rows.append({
                    "Algorithm":       r.algorithm_name,
                    "Data Size":       r.data_size,
                    "Data Type":       str(r.data_type),
                    "Elapsed (ms)":    round(r.elapsed_ms, 4),
                    "Median (ms)":     round(r.median_elapsed_ms, 4),
                    "StdDev (ms)":     round(r.stddev_elapsed_ms, 4),
                    "Key Ops":         r.key_op_count,
                    "Compares":        r.compare_count,
                    "Swaps":           r.swap_count,
                    "Moves":           r.move_count,
                    "Timed Out":       r.timed_out,
                    "Correct":         r.sorted_correctly,
                })

            df = pd.DataFrame(rows)

            # Store for display & export
            st.session_state["df_results"] = df
            st.session_state["ran"] = True

            # Also save raw data for potential debugging
            st.session_state["raw_results"] = raw_results

# ============================================================
# Display results
# ============================================================
if st.session_state.get("ran") and "df_results" in st.session_state:
    df: pd.DataFrame = st.session_state["df_results"]

    # ---- 2. Results table ----------------------------------------------------
    st.subheader("📊 Benchmark Results")

    # Colour-code the Correct column
    def _highlight_correct(val: bool) -> str:
        if val is True:
            return "color: #16a34a; font-weight: bold"
        if val is False:
            return "color: #dc2626; font-weight: bold"
        return ""

    styled = df.style.map(_highlight_correct, subset=["Correct"]).format(
        {
            "Elapsed (ms)":  "{:.4f}",
            "Median (ms)":   "{:.4f}",
            "StdDev (ms)":   "{:.4f}",
            "Key Ops":       "{:,.0f}",
            "Compares":      "{:,.0f}",
            "Swaps":         "{:,.0f}",
            "Moves":         "{:,.0f}",
        },
        na_rep="-",
    )
    st.dataframe(styled, use_container_width=True, hide_index=True)

    # ---- Quick stats ----------------------------------------------------------
    c1, c2, c3, c4 = st.columns(4)
    with c1:
        fastest = df.loc[df["Elapsed (ms)"].idxmin()]
        st.metric(
            "🚀 Fastest",
            str(fastest["Algorithm"]),
            f"{fastest['Elapsed (ms)']:.3f} ms",
        )
    with c2:
        slowest = df.loc[df["Elapsed (ms)"].idxmax()]
        st.metric(
            "🐢 Slowest",
            str(slowest["Algorithm"]),
            f"{slowest['Elapsed (ms)']:.3f} ms",
        )
    with c3:
        fewest = df.loc[df["Key Ops"].idxmin()]
        st.metric(
            "⚡ Fewest Key Ops",
            str(fewest["Algorithm"]),
            f"{fewest['Key Ops']:,.0f}",
        )
    with c4:
        all_ok = df["Correct"].all()
        any_timeout = df["Timed Out"].any() if "Timed Out" in df.columns else False
        if all_ok and not any_timeout:
            st.metric("✅ All Correct", "Yes")
        elif any_timeout:
            st.metric("⏰ Timeouts", f"{df['Timed Out'].sum()} / {len(df)}")
        else:
            st.metric("❌ Failures", f"{(~df['Correct']).sum()} / {len(df)}")

    # ---- 3. Time bar chart ---------------------------------------------------
    st.subheader("⏱️ Execution Time by Algorithm")
    fig_time = px.bar(
        df,
        x="Algorithm",
        y="Elapsed (ms)",
        color="Algorithm",
        text=df["Elapsed (ms)"].apply(lambda v: f"{v:.3f}"),
        error_y=df["StdDev (ms)"] if df["StdDev (ms)"].sum() > 0 else None,
        title=(
            f"Average Execution Time — "
            f"{st.session_state.get('last_data_type', '?')} data, "
            f"{st.session_state.get('last_data_size', '?')} elements, "
            f"{st.session_state.get('last_repeat', '?')} repeat(s)"
        ),
    )
    fig_time.update_traces(textposition="outside")
    fig_time.update_layout(showlegend=False, yaxis_title="Elapsed (ms)")
    st.plotly_chart(fig_time, use_container_width=True)

    # ---- 4. Key Ops bar chart -------------------------------------------------
    st.subheader("🔢 Key Operations by Algorithm")
    fig_ops = px.bar(
        df,
        x="Algorithm",
        y="Key Ops",
        color="Algorithm",
        text=df["Key Ops"].apply(lambda v: f"{v:,.0f}"),
        title=(
            f"Key Operations Count — "
            f"{st.session_state.get('last_data_type', '?')} data, "
            f"{st.session_state.get('last_data_size', '?')} elements"
        ),
    )
    fig_ops.update_traces(textposition="outside")
    fig_ops.update_layout(showlegend=False, yaxis_title="Key Operations")
    st.plotly_chart(fig_ops, use_container_width=True)

    # ---- Comparison breakdown (optional extra) --------------------------------
    with st.expander("📋 Operation Breakdown (Compare / Swap / Move)"):
        fig_detail = px.bar(
            df.melt(
                id_vars=["Algorithm"],
                value_vars=["Compares", "Swaps", "Moves"],
                var_name="Operation",
                value_name="Count",
            ),
            x="Algorithm",
            y="Count",
            color="Operation",
            barmode="group",
            title="Operation Count Breakdown by Algorithm",
        )
        st.plotly_chart(fig_detail, use_container_width=True)

    # ---- 5. CSV Export --------------------------------------------------------
    st.subheader("💾 Export Results")
    csv_bytes: bytes = df.to_csv(index=False).encode("utf-8")
    ts: str = time.strftime("%Y%m%d_%H%M%S")
    fname: str = (
        f"benchmark_{st.session_state.get('last_data_type', 'unknown')}_"
        f"{st.session_state.get('last_data_size', 0)}_{ts}.csv"
    )
    st.download_button(
        label="📥 Download CSV",
        data=csv_bytes,
        file_name=fname,
        mime="text/csv",
    )
    st.caption(f"`{fname}` — {len(df)} rows × {len(df.columns)} columns")

# ============================================================
# 6. Smart Selector Recommendation  (Member C)
# ============================================================
st.divider()
st.subheader("🤖 Smart Selector Recommendation")

if "last_data" in st.session_state:
    try:
        from ml.recommend import recommend

        rec: dict = recommend(st.session_state["last_data"])

        col_a, col_b = st.columns(2)
        with col_a:
            st.metric("⭐ Recommended Algorithm", str(rec.get("algorithm", "N/A")))
        with col_b:
            conf = rec.get("confidence", 0.0)
            st.metric("📈 Confidence", f"{float(conf):.1%}")

        reason = rec.get("reason", rec.get("explanation", ""))
        if reason:
            st.info(f"**Why this choice:** {reason}")

        if rec.get("fallback"):
            st.caption(f"Fallback: {rec['fallback']}")

    except ImportError:
        st.info(
            "💡 **ML recommendation module not available yet.**  "
            "`ml/recommend.py` will be delivered by Member C.  "
            "Once available, this block will automatically populate."
        )
    except Exception as exc:
        st.warning(f"Recommendation lookup raised an error: {exc}")
else:
    st.info("👆 Run a benchmark first, then a recommendation will appear here.")
