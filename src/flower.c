#include "flower.h"
#include <float.h>
#include <math.h>
#include <string.h>

#define PI 3.14159265358979323846f
static FlowerPoint add(FlowerPoint a, FlowerPoint b) { return (FlowerPoint){a.x+b.x,a.y+b.y,a.z+b.z}; }
static FlowerPoint sub(FlowerPoint a, FlowerPoint b) { return (FlowerPoint){a.x-b.x,a.y-b.y,a.z-b.z}; }
static FlowerPoint scale(FlowerPoint a, float s) { return (FlowerPoint){a.x*s,a.y*s,a.z*s}; }
static float dot(FlowerPoint a, FlowerPoint b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
static FlowerPoint cross(FlowerPoint a, FlowerPoint b) { return (FlowerPoint){a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
static int vertex(int ring, int slice) { return ring*FLOWER_SLICES+(slice%FLOWER_SLICES); }
static bool valid_hit(FlowerHit hit) {
    return hit.hit && isfinite(hit.radial) && isfinite(hit.angle) && hit.radial>=0 && hit.radial<=1;
}
static void link_add(Flower *flower, int first, int second, float stiffness) {
    FlowerPoint delta=sub(flower->reference[second],flower->reference[first]);
    FlowerPoint middle=add(flower->reference[first],flower->reference[second]);
    float radius_squared=middle.x*middle.x+middle.z*middle.z;
    float length_squared=dot(delta,delta);
    float tangent=radius_squared>0 ? (-delta.x*middle.z+delta.z*middle.x)/sqrtf(radius_squared) : 0;
    FlowerLink *link=&flower->link[flower->link_count++];
    *link=(FlowerLink){(uint16_t)first,(uint16_t)second,sqrtf(length_squared),
                       fminf(1,tangent*tangent/length_squared),stiffness};
}
void flower_init(Flower *flower) {
    memset(flower,0,sizeof(*flower));
    for (int ring=0;ring<=FLOWER_RINGS;ring++) for (int slice=0;slice<FLOWER_SLICES;slice++) {
        int index=vertex(ring,slice);
        float radial=(float)ring/FLOWER_RINGS, angle=2*PI*slice/FLOWER_SLICES;
        float radius=0.22f+radial*(1.5f+0.32f*cosf(5*angle));
        /* Tiny deterministic imperfections break symmetry; they are not animated ruffles. */
        uint32_t random=(uint32_t)(index+1)*UINT32_C(2654435761);
        float seed=((float)(random>>16)/65535.0f-0.5f)*0.006f*radial;
        FlowerPoint point={radius*cosf(angle),0.16f*radial*radial+seed,radius*sinf(angle)};
        flower->reference[index]=flower->position[index]=point;
    }
    int triangle=0;
    for (int ring=0;ring<=FLOWER_RINGS;ring++) for (int slice=0;slice<FLOWER_SLICES;slice++) {
        int first=vertex(ring,slice), next=vertex(ring,slice+1);
        link_add(flower,first,next,0.8f);
        link_add(flower,first,vertex(ring,slice+2),0.035f);
        if (ring<FLOWER_RINGS) {
            int outer=vertex(ring+1,slice), diagonal=vertex(ring+1,slice+1);
            link_add(flower,first,outer,0.8f);
            link_add(flower,first,diagonal,0.65f);
            link_add(flower,next,outer,0.65f);
            uint16_t indices[6]={(uint16_t)first,(uint16_t)next,(uint16_t)outer,
                                  (uint16_t)next,(uint16_t)diagonal,(uint16_t)outer};
            memcpy(flower->triangle+triangle,indices,sizeof(indices)); triangle+=6;
        }
        if (ring<FLOWER_RINGS-1) link_add(flower,first,vertex(ring+2,slice),0.035f);
    }
}
void flower_hold_init(FlowerHold *hold) { *hold=(FlowerHold){.owner=-1}; }
void flower_hold_cancel(FlowerHold *hold) { *hold=(FlowerHold){.blocked=true,.owner=-1}; }
void flower_hold_all_released(FlowerHold *hold) { flower_hold_init(hold); }
bool flower_hold_begin(FlowerHold *hold, int pointer, bool fresh_press, bool focused, FlowerHit hit) {
    if (hold->captured || hold->blocked || !fresh_press || !focused || !valid_hit(hit)) return false;
    *hold=(FlowerHold){.captured=true,.owner=pointer};
    return true;
}
void flower_hold_release(FlowerHold *hold, int pointer) {
    if (hold->captured && hold->owner==pointer) flower_hold_cancel(hold);
}
static void simulation_step(Flower *flower, FlowerHit hit) {
    const float brush_radius=0.38f;
    for (int ring=1;ring<=FLOWER_RINGS;ring++) for (int slice=0;slice<FLOWER_SLICES;slice++) {
        int index=vertex(ring,slice);
        float radial=(float)ring/FLOWER_RINGS, angle=2*PI*slice/FLOWER_SLICES;
        float angle_delta=remainderf(angle-hit.angle,2*PI);
        float across=angle_delta*(0.22f+1.5f*(radial+hit.radial)*0.5f);
        float along=(radial-hit.radial)*1.82f;
        float distance_squared=across*across+along*along;
        if (distance_squared<brush_radius*brush_radius) {
            float weight=1-distance_squared/(brush_radius*brush_radius);
            /* Tangential preferred length grows locally; root and distant material do not. */
            float gain=0.28f*(float)FLOWER_STEP*weight*weight*(0.2f+0.8f*radial*radial);
            flower->growth[index]=fminf(1.5f,flower->growth[index]+gain);
        }
    }
    /* Spring-sheet approximation, not a calibrated shell or collision solver. */
    for (int pass=0;pass<8;pass++) for (int order=0;order<flower->link_count;order++) {
        int index=(pass&1)? flower->link_count-1-order : order;
        FlowerLink link=flower->link[index];
        float growth=0.5f*(flower->growth[link.first]+flower->growth[link.second]);
        float preferred=link.length*sqrtf(1+((1+growth)*(1+growth)-1)*link.tangent_squared);
        FlowerPoint delta=sub(flower->position[link.second],flower->position[link.first]);
        float distance=sqrtf(dot(delta,delta));
        int first_free=link.first>=FLOWER_SLICES, second_free=link.second>=FLOWER_SLICES;
        int free_count=first_free+second_free;
        if (distance<1e-8f || !free_count) continue;
        FlowerPoint correction=scale(delta,link.stiffness*(distance-preferred)/(distance*free_count));
        if (first_free) flower->position[link.first]=add(flower->position[link.first],correction);
        if (second_free) flower->position[link.second]=sub(flower->position[link.second],correction);
    }
    flower->steps++;
}
bool flower_hold_frame(Flower *flower, FlowerHold *hold, int pointer, bool down,
                       bool focused, FlowerHit hit, double frame_seconds) {
    if (!focused || !isfinite(frame_seconds) || frame_seconds<0 || frame_seconds>FLOWER_FRAME_LIMIT) {
        flower_hold_cancel(hold); return false;
    }
    if (!hold->captured || hold->blocked || pointer!=hold->owner) return false;
    if (!down) { flower_hold_release(hold,pointer); return false; }
    if (!valid_hit(hit)) { hold->remainder=0; return false; }
    hold->remainder+=fmin(frame_seconds,FLOWER_CATCHUP_LIMIT);
    bool changed=false;
    while (hold->remainder+1e-12>=FLOWER_STEP) {
        simulation_step(flower,hit);
        hold->remainder-=FLOWER_STEP;
        if (hold->remainder<0) hold->remainder=0;
        changed=true;
    }
    return changed;
}
FlowerHit flower_pick(const Flower *flower, FlowerPoint origin, FlowerPoint direction) {
    FlowerHit best={.distance=FLT_MAX};
    for (int index=0;index<FLOWER_TRIANGLES*3;index+=3) {
        int first=flower->triangle[index], second=flower->triangle[index+1], third=flower->triangle[index+2];
        FlowerPoint a=flower->position[first], edge1=sub(flower->position[second],a), edge2=sub(flower->position[third],a);
        FlowerPoint p=cross(direction,edge2);
        float determinant=dot(edge1,p);
        if (fabsf(determinant)<1e-8f) continue;
        float inverse=1/determinant;
        FlowerPoint from_a=sub(origin,a);
        float weight1=dot(from_a,p)*inverse;
        if (weight1<0 || weight1>1) continue;
        FlowerPoint q=cross(from_a,edge1);
        float weight2=dot(direction,q)*inverse;
        if (weight2<0 || weight1+weight2>1) continue;
        float distance=dot(edge2,q)*inverse;
        if (distance<=0 || distance>=best.distance) continue;
        float weight0=1-weight1-weight2;
        FlowerPoint material=add(scale(flower->reference[first],weight0),
                              add(scale(flower->reference[second],weight1),scale(flower->reference[third],weight2)));
        float radial=(weight0*(first/FLOWER_SLICES)+weight1*(second/FLOWER_SLICES)+weight2*(third/FLOWER_SLICES))/FLOWER_RINGS;
        best=(FlowerHit){true,radial,atan2f(material.z,material.x),distance,add(origin,scale(direction,distance))};
    }
    return best;
}
void flower_normals(const Flower *flower, FlowerPoint *normals) {
    memset(normals,0,sizeof(*normals)*FLOWER_VERTICES);
    for (int index=0;index<FLOWER_TRIANGLES*3;index+=3) {
        int a=flower->triangle[index], b=flower->triangle[index+1], c=flower->triangle[index+2];
        FlowerPoint normal=cross(sub(flower->position[b],flower->position[a]),sub(flower->position[c],flower->position[a]));
        normals[a]=add(normals[a],normal); normals[b]=add(normals[b],normal); normals[c]=add(normals[c],normal);
    }
    for (int index=0;index<FLOWER_VERTICES;index++) {
        float length=sqrtf(dot(normals[index],normals[index]));
        normals[index]=length>1e-8f? scale(normals[index],1/length):(FlowerPoint){0,1,0};
    }
}
