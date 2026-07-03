#pragma once

/// Thrown by TraceArray operations when a timeout has fired.
/// Not derived from std::exception — this keeps it distinct from
/// unrelated runtime errors so it cannot be swallowed by catch(...).
struct SortCancelledException {};
