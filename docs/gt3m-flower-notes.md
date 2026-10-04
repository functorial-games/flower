# GT3M notes for Flower

Source: William P. Thurston, *The Geometry and Topology of Three-Manifolds*,
electronic version 1.1 (March 2002), based on the 1980 Princeton notes.

Official source and chapter downloads:
https://library.slmath.org/nonmsri/gt3m/

This file is a Flower design note, not a copy of Thurston's chapters. It
paraphrases the parts we are using and points back to the source.

## Chapter 1: geometry and topology belong together

Chapter 1, section 1.1 opens with the theme that

> "topology and geometry, in dimensions up through 3, are very intricately related."

For Flower, take that separation seriously without treating it as a wall:

- topology says which local pieces belong together;
- geometry says their current intrinsic sizes, angles and preferred distances;
- an embedding says where the material currently sits in 3-space;
- rendering samples that state into whatever triangles raylib needs.

The triangle mesh is therefore not the identity of the flower. It is derived
drawing data and may be discarded and regenerated.

Direct chapter 1 PDF:
https://library.slmath.org/nonmsri/gt3m/PDF/1.pdf

## Chapter 3: local pieces and gluing data

Chapter 3 starts from a manifold as a topological space locally modelled on
Euclidean space. Thurston then describes additional geometric structure by
specifying which local homeomorphisms are allowed when open pieces are glued
together. The allowed local maps are required to behave coherently under
restriction, composition, inversion and local assembly.

Flower does not need to imitate Thurston's full (G,X)-manifold machinery.
The useful architectural idea is that the global surface can be represented
through local open patches and overlap/gluing information instead of making a
render mesh authoritative.

Direct chapter 3 PDF:
https://library.slmath.org/nonmsri/gt3m/PDF/3.pdf

## Flower's topological net

Start with a finite working cover

    U = { U_0, U_1, ..., U_(n-1) }

of local flower patches.

Store overlap as a relation

    i ~ j  iff  U_i intersects U_j.

This gives a small combinatorial net for traversal. Higher intersections can
also be recorded when needed; pairwise adjacency alone is not always enough
to reconstruct the topology.

For a selected patch i, define hop neighborhoods by the overlap relation:

    H_0(i) = { i }

    H_(k+1)(i) = H_k(i) union
                 { j : j ~ q for some q in H_k(i) }.

This is topological/material distance, not Euclidean distance in the rendered
3-D scene. Two folded petals may be millimetres apart in space while many
material hops apart.

The implementation may lay adjacent patches near one another in memory for
cache locality, but memory adjacency must not define topology.

## Growth while held

A pointer press identifies one or more local patches. While that same pointer
continues to hold the selected material region:

1. find the selected patch set;
2. expand to a bounded hop neighborhood;
3. modify intrinsic growth data in those patches;
4. update the embedding/deformation;
5. regenerate only drawing data that became stale.

On release, cancellation or loss of the owning pointer, steps 3 and 4 stop.
The flower remains at its current state. Camera motion must not advance growth.

"Adding surface area" should initially mean changing intrinsic preferred
geometry, not inserting render triangles. Mesh refinement may happen
independently when more sampling resolution is useful.

## State split

The retained flower state should eventually distinguish at least:

    TOPOLOGY
        patch identities
        overlap / gluing relations
        boundary relationships

    INTRINSIC GEOMETRY
        preferred local metric or equivalent rest-length data
        accumulated growth state

    EMBEDDING / CONFIGURATION
        enough current deformation state to continue deterministically

    RENDER CACHE
        sampled positions
        normals
        triangle indices
        GPU buffers

The first three describe the flower. The last one describes one current way
to draw it.

## First implementation consequence

Do not rewrite the current prototype merely to rename its triangle mesh
"topology." Instead, introduce the topological cover as an authoritative layer
and make the existing spring sheet / raylib mesh an adapter derived from it.

The first useful invariant is:

    changing render resolution must not change patch identity,
    hop distance, selected growth region, or accumulated intrinsic growth.

That gives us a testable boundary between topology and rendering before we
attempt a more sophisticated elastic surface model.
