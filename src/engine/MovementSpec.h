#pragma once

// Value Object — mechanical properties of the watch movement under test.
// bph and liftAngle together identify the movement type; two movements with
// the same values are interchangeable from the measurement system's perspective.
// Immutable once set at session start (UI controls are disabled during a session).
struct MovementSpec {
    int    bph       = 28800; // beats per hour (0 = auto-detect)
    double liftAngle = 52.0;  // balance wheel lift angle in degrees
};
