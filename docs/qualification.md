# Qualification receipt — 2026-10-04

## Executed locally

The actual `src/flower.c` implementation was compiled and executed with host C11, `-O2 -DNDEBUG -Wall -Wextra -Werror -pedantic`. This is diagnostic execution of shared code, not an Android product build.

Passing regression groups:

1. Starts paused, including hundreds of idle frames.
2. No fresh, focused, on-surface press means no growth.
3. Held material gains preferred length and changes geometry; distant growth, root positions and connectivity stay unchanged.
4. Release freezes the entire Flower state bit-for-bit over hundreds of frames.
5. No pointer handoff; a released and rearmed fresh press resumes.
6. An unrelated pointer neither grows nor releases the captured stroke.
7. Off-surface time does not accumulate.
8. Focus loss cancels and requires a fresh stroke.
9. Explicit touch cancellation freezes state.
10. Release discards fractional fixed-step time; no paused-time catch-up.
11. Invalid or stalled frame times cancel instead of growing a backlog.
12. Invalid material coordinates cannot mutate the flower.
13. Equal hold duration at 60 Hz and 120 Hz yields identical full state.
14. Actual two-sided triangle picking, including empty background and the center hole.
15. Derived normals remain finite and unit-length.

The mesh uses 1,088 vertices, 2,048 triangles and 6,208 links. Four deliberately broken implementations must fail runtime checks, not merely fail compilation.

## Not established by those tests

The raylib window/input/rendering adapter has not been executed locally. Its Android lifecycle hooks and screen-to-material picking are not proven by the isolated core tests. The cloud NDK workflow must qualify actual compilation and linking. Separate rendered-input and lifecycle tests must still prove Android release/cancel/reentry and simultaneous camera/growth controls.

No APK has been packaged or signed, and no physical device was touched. MIRO A1 rendering, frame rate, memory consumption, multitouch behavior and replacement compatibility remain unverified. No screenshot, performance claim or successful phone run is implied by a passing core test.

## Dependencies and representation

Raylib is pinned to `c1ab645ca298a2801097931d1079b10ff7eb9df8` (5.5). The Android host wraps that revision's native app callbacks, forwarding every callback to raylib; it does not modify or replace raylib's platform implementation. An upgrade must requalify that seam.

The pigment is constant; brightness is a derived, two-sided normal-based inspection shade. Growth changes preferred tangential link distances in material coordinates. Weak two-hop links are an approximation, not an independently validated bending constitutive law. Lua and generated GPU/CPU kernels remain future execution backends, not implemented features.
