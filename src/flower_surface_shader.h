#ifndef FLOWER_SURFACE_SHADER_H
#define FLOWER_SURFACE_SHADER_H

/* Mostow-inspired front/back lighting and intrinsic rings.
   Uses raylib's bound attribute and matrix uniform names on desktop and GLES2.
   The reference material coordinate is supplied by FlowerMesh, not recomputed
   from changing world-space positions or camera state. */
#if defined(PLATFORM_ANDROID)
#define FLOWER_V_VERSION "#version 100\n"
#define FLOWER_V_IN "attribute"
#define FLOWER_V_OUT "varying"
#define FLOWER_F_VERSION "#version 100\nprecision mediump float;\n"
#define FLOWER_F_IN "varying"
#define FLOWER_F_DECL ""
#define FLOWER_F_RESULT "gl_FragColor"
#else
#define FLOWER_V_VERSION "#version 330\n"
#define FLOWER_V_IN "in"
#define FLOWER_V_OUT "out"
#define FLOWER_F_VERSION "#version 330\n"
#define FLOWER_F_IN "in"
#define FLOWER_F_DECL "out vec4 finalColor;\n"
#define FLOWER_F_RESULT "finalColor"
#endif

static const char flower_surface_vertex_shader[] =
    FLOWER_V_VERSION
    FLOWER_V_IN " vec3 vertexPosition;\n"
    FLOWER_V_IN " vec3 vertexNormal;\n"
    FLOWER_V_IN " vec2 vertexTexCoord;\n"
    "uniform mat4 mvp;\n"
    "uniform mat4 matNormal;\n"
    FLOWER_V_OUT " vec3 v_normal;\n"
    FLOWER_V_OUT " float v_material_radius;\n"
    "void main(){\n"
    "  gl_Position=mvp*vec4(vertexPosition,1.0);\n"
    "  v_normal=(matNormal*vec4(vertexNormal,0.0)).xyz;\n"
    "  v_material_radius=vertexTexCoord.x;\n"
    "}\n";

static const char flower_surface_fragment_shader[] =
    FLOWER_F_VERSION
    FLOWER_F_IN " vec3 v_normal;\n"
    FLOWER_F_IN " float v_material_radius;\n"
    "uniform float u_rings;\n"
    FLOWER_F_DECL
    "void main(){\n"
    "  vec3 n=normalize(v_normal);\n"
    "  if (!gl_FrontFacing) n=-n;\n"
    "  float light=0.35+0.60*max(0.0,dot(n,normalize(vec3(-0.4,0.6,0.9))));\n"
    "  vec3 material=gl_FrontFacing?vec3(0.879,0.392,0.545):vec3(0.46,0.65,0.67);\n"
    "  float band=1.0-smoothstep(0.0,0.12,abs(sin(v_material_radius*12.5663706)));\n"
    "  material=mix(material,vec3(0.26,0.32,0.30),band*0.45*u_rings);\n"
    "  " FLOWER_F_RESULT "=vec4(material*light,1.0);\n"
    "}\n";
#endif
