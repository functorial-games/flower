#include "flower.h"
#include "flower_surface_shader.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* CMake runs this with NDEBUG: do not use assert(). */
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr,"surface cache check failed line %d: %s\n",__LINE__,#condition); \
    return 1; \
} } while (0)

static Flower flower;

int main(void)
{
    flower_init(&flower);
    Flower *before=malloc(sizeof(*before));
    CHECK(before != NULL);
    memcpy(before,&flower,sizeof(*before));
    FlowerMesh coarse={0},fine={0};
    CHECK(flower_mesh_build(&flower,1,&coarse));
    CHECK(flower_mesh_build(&flower,2,&fine));
    CHECK(coarse.surface_count==FLOWER_TRIANGLES);
    CHECK(fine.surface_count==coarse.surface_count);
    CHECK(fine.triangle_count==4*coarse.triangle_count);
    CHECK(coarse.material_radius != NULL && fine.material_radius != NULL);
    for (int parent=0;parent<coarse.surface_count;parent++) {
        const FlowerSurfaceTriangle *source=&coarse.surface[parent];
        CHECK(coarse.parent[parent]==parent);
        CHECK(fine.parent[4*parent]==parent);
        for (int vertex=0;vertex<3;vertex++)
            CHECK(fabsf(coarse.material_radius[3*parent+vertex]-source->radial[vertex])<1e-6f);
        /* The subdivided display has the original triangle's three corners. */
        CHECK(fabsf(fine.material_radius[12*parent]-source->radial[0])<1e-6f);
        CHECK(fabsf(fine.material_radius[12*parent+4]-source->radial[1])<1e-6f);
        CHECK(fabsf(fine.material_radius[12*parent+8]-source->radial[2])<1e-6f);
    }
    for (int vertex=0;vertex<fine.vertex_count;vertex++) {
        float radius=fine.material_radius[vertex];
        CHECK(isfinite(radius) && radius>=-1e-6f && radius<=1.0f+1e-6f);
    }
    CHECK(memcmp(before,&flower,sizeof(*before))==0);
    CHECK(strstr(flower_surface_vertex_shader,"vertexTexCoord")!=NULL);
    CHECK(strstr(flower_surface_vertex_shader,"matNormal")!=NULL);
    CHECK(strstr(flower_surface_fragment_shader,"gl_FrontFacing")!=NULL);
    CHECK(strstr(flower_surface_fragment_shader,"u_rings")!=NULL);
    flower_mesh_free(&coarse);
    flower_mesh_free(&fine);
    free(before);
    return 0;
}
