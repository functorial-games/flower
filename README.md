# Flower: hold to grow

Native raylib prototype. **Press and hold a region of the flower to grow it. Release to freeze its exact current shape.** Starts paused. Camera motion never advances flower time. No play/pause toggle, automatic growth, or spacebar growth binding.

## Implemented source

- A single fixed rose-colored pigment, with simple two-sided normal-based shading.
- A triangulated, five-lobed sheet with a fixed inner ring. Holding increases preferred tangential material distances inside a bounded material-space brush. A spring-sheet relaxation changes the geometry; adding triangles is not growth.
- A captured pointer ID owns each stroke. Release, Android touch cancellation, focus loss, or a long stall cancels it. An already-down finger cannot take over another finger's stroke. After cancellation, lift all contacts before starting a fresh stroke.
- Missed surface or crossing into camera controls pauses growth without accumulating time. While held, moving the contact moves the material-space brush. Mechanical deformation can spread beyond the region whose preferred distances increase.
- The solver and material growth both stop on release: no residual settling. Ordinary slow frames execute at most 50 ms of growth; a gap over 250 ms cancels the hold. Discarded time never catches up.
- Independent camera translation, looking and roll. Android has separate FLY/LOOK touch pads and UP/DOWN/ROLL controls. Desktop inspection uses WASD, Q/E, right-drag and Z/X. The spacebar does nothing.

## State and execution

`src/flower.c` owns retained reference positions, current positions, local growth and fixed connectivity. The hold controller owns the small fixed-step remainder; the camera is separate. The renderer derives normals and color/vertex buffers only when the geometry changes. It contains no time-driven deformation shader.

Arrays and operations form the native boundary. This implementation does not require future procedural patches, brush strokes or generated kernels to map one-to-one onto C records. Lua and custom GPU/assembly generation are **not wired in this first slice**; the growth gate is native and cannot be bypassed by a future scripted rule. No persistence across process death yet.

## Limits of the shape model

This is a spring-sheet approximation with strong neighbor links and weak two-hop links, not a calibrated elastic-shell or plant model. It does not include self-collision, petal contact, thickness, translucency or anatomical petals. Visual quality and device performance still need qualification. The tests demonstrate local preferred-length growth and changed geometry, not scientifically validated flower morphogenesis.

## Build and evidence

Product route: Android NDK r27c (`27.2.12479018`), `armeabi-v7a`, API 24 minimum, pinned raylib 5.5 source `c1ab645ca298a2801097931d1079b10ff7eb9df8`. Intended phone: MIRO A1, Android 14. The initially empty repository supplied no qualified Ick/raylib/NativeActivity build integration; this is the explicit Ick integration gap, not a claim that Ick cannot compile this code.

`CMakeLists.txt` builds a native shared library through the NDK. It does **not** package or sign an APK. No package identity, signer, device-install or replacement evidence is asserted. A host viewer requires the explicit diagnostic option `FLOWER_HOST_DIAGNOSTIC=ON`; it is not the product build route.

`sh scripts/test.sh` runs the actual shared C simulation/input gate on the host with `NDEBUG`, explicit failing checks, and compiler warnings as errors. `python3 scripts/mutation_test.py` requires runtime rejection of ignored release, ignored pointer identity, ignored focus loss and a disconnected simulation. These are diagnostic tests, not phone acceptance. The workflow separately compiles/links the real raylib Android host and records the unsigned library hash.

See `docs/qualification.md` for the evidence boundary. The assistant is responsible for implementation; these commands document repeatable checks, not work handed back to the player.
