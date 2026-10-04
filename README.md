# Flower

An interactive, single-color flower. Use raylib as the initial rendering and input host.

The player holds a region of the flower to grow it. Releasing pauses the flower at its exact current shape. Camera flight, looking and rolling remain independent of growth. Paused time must never accumulate into later growth.

Growth means locally increasing preferred material distances, not adding triangles or playing a timed wave animation. The representation may combine retained data, computation and drawing caches.
