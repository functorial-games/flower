# Mostow → Flower: surface rendering

Origin: [Flexible Pipes #87](https://github.com/isomorphisms/flexible-pipes/issues/87).
Reference: `isomorphismes/mostow@a33da13baac446c7b6a22862f41aed444129b3ca`,
`android/mostow_renderer.c` and `native/mostow_view.c`.

This consumer keeps Flower's Net → Skin → Mesh and raylib 5.5 shell. It uses
world-space normal lighting and two-sided front/back colors adapted from Mostow,
plus an optional intrinsic ring overlay. Flower's smooth area-weighted vertex
normals are retained. No hyperbolic distance, runtime geometry optimization,
Mostow's animation modes, or Mostow Android shell was copied.

The display cache now interpolates `material_radius` from the same canonical
parent face chart as its positions and normals. It is not growth state. The raylib
mesh's first UV component carries it to the shader (rather than using ambient
XY/world-space radius, which would slide when the sheet folds). Drawing density
1 versus 2 cannot redefine it or create material neighbors. Unit tests prove
original triangle corners and material state are unchanged.

The new shader accepts the default raylib position, normal and UV attributes
and `mvp`/`matNormal` uniforms. Separate GLSL 100 and 330 sources use the same
C interface. Shader link and semantic locations are checked at application
startup; silent fallback to a default shader is rejected. Android and desktop
rendering must still be exercised on actual GL drivers before qualification.

The **RINGS** touch control / desktop `R` key toggles the material-space overlay;
the existing mesh-density control is unchanged. Other gesture semantics and
hold-growth ownership are unchanged.

Scope of source acceptance: host C checks and source review. Android A1/C67
GLES2 rendering, touch/lifecycle acceptance, performance, and APK producer
authentication remain **NOT_VERIFIED**. The existing ICK/Bionic application
compilation gap and registered release requirements remain separate blockers.
