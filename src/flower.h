#ifndef FLOWER_H
#define FLOWER_H
#include <stdbool.h>
#include <stdint.h>

#define FLOWER_RINGS 16
#define FLOWER_SLICES 64
#define FLOWER_VERTICES ((FLOWER_RINGS + 1)*FLOWER_SLICES)
#define FLOWER_TRIANGLES (2*FLOWER_RINGS*FLOWER_SLICES)
#define FLOWER_MAX_EDGES ((6*FLOWER_RINGS + 2)*FLOWER_SLICES)
#define FLOWER_STEP (1.0/120.0)
#define FLOWER_FRAME_LIMIT 0.25
#define FLOWER_CATCHUP_LIMIT 0.05

typedef struct { float x, y, z; } FlowerPoint;
typedef struct { uint16_t first, second; float length, tangent_squared, stiffness; } FlowerLink;
/* Material state is separate from input/camera state and from GPU buffers. */
typedef struct {
    FlowerPoint position[FLOWER_VERTICES], reference[FLOWER_VERTICES];
    float growth[FLOWER_VERTICES];
    uint16_t triangle[FLOWER_TRIANGLES*3];
    FlowerLink link[FLOWER_MAX_EDGES];
    int link_count;
    uint64_t steps;
} Flower;
typedef struct { bool hit; float radial, angle, distance; FlowerPoint point; } FlowerHit;
typedef struct {
    bool captured, blocked;
    int owner;
    double remainder;
} FlowerHold;

void flower_init(Flower *flower);
void flower_hold_init(FlowerHold *hold);
void flower_hold_cancel(FlowerHold *hold);
/* A fresh press is mandatory. A held pointer cannot inherit another's stroke. */
bool flower_hold_begin(FlowerHold *hold, int pointer, bool fresh_press, bool focused, FlowerHit hit);
void flower_hold_release(FlowerHold *hold, int pointer);
void flower_hold_all_released(FlowerHold *hold);
/* Release, missed surface, focus loss and invalid/stalled time never step the flower. */
bool flower_hold_frame(Flower *flower, FlowerHold *hold, int pointer, bool down,
                       bool focused, FlowerHit hit, double frame_seconds);
FlowerHit flower_pick(const Flower *flower, FlowerPoint origin, FlowerPoint direction);
void flower_normals(const Flower *flower, FlowerPoint *normals);
#endif
