#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

#define kEmpty  0xffffffff
#define kGray   (float3) (0.5, 0.5, 0.5)

/** ---------------------------------------------------------------------------
 * Point data type.
 */
typedef struct {
    float3 pos;
    float3 col;
} Point_t;

/** KeyValue data type. */
typedef struct {
    uint key;
    uint value;
} KeyValue_t;

/** Hashmap hash function. */
uint hash(const uint3 v);

/** ---------------------------------------------------------------------------
 * hash
 * Hashmap hash function.
 */
uint hash(const uint3 v)
{
    const uint c1 = 73856093;
    const uint c2 = 19349663;
    const uint c3 = 83492791;

    uint h1 = c1 * v.s0;
    uint h2 = c2 * v.s1;
    uint h3 = c3 * v.s2;

    return (h1 ^ h2 ^ h3);
    // return (7*h1 + 503*h2 + 24847*h3);
}

/** ---------------------------------------------------------------------------
 * hashmap_clear
 * Clear the hashmap.
 */
__kernel void hashmap_clear(
    __global KeyValue_t *hashmap,
    const uint capacity)
{
    const uint id = get_global_id(0);
    if (id < capacity) {
        hashmap[id].key = kEmpty;
    }
}

/** ---------------------------------------------------------------------------
 * hashmap_build
 * Insert a set of points into the hashmap KeyValue array.
 * For each point, compute the index coordinates of the cell that contains it.
 * Compute the hash key associated with the cell and its corresponding slot
 * in the hashmap.
 * Linearly probe the map and insert the key in the first empty slot marked
 * as kEmpty. When found, store the point id in the slot value.
 */
__kernel void hashmap_build(
    __global KeyValue_t *hashmap,
    const uint capacity,
    const __global Point_t *points,
    const uint n_points,
    const uint n_cells,
    const float3 domain_lo,
    const float3 domain_hi)
{
    const uint id = get_global_id(0);
    if (id < n_points) {
        float3 u_pos = (points[id].pos - domain_lo) / (domain_hi - domain_lo);
        uint3 cell_ix = (uint3) (
            (float) n_cells * u_pos.x,
            (float) n_cells * u_pos.y,
            (float) n_cells * u_pos.z);

        uint key = hash(cell_ix);
        uint slot = key % capacity;
        while (true) {
            uint prev = atomic_cmpxchg(
                (volatile __global unsigned int *) (&hashmap[slot].key),
                kEmpty,
                key);

            if (prev == kEmpty) {
                hashmap[slot].value = id;
                return;
            }

            slot = (slot + 1) % capacity;   // & (capacity - 1);
        }
    }
}

/** --------------------------------------------------------------------------
 * hashmap_query
 * Query the hashamp for the set points that belong to the same cell as the
 * probe. Color each point accordingly.
 */
__kernel void hashmap_query(
    __global Point_t *points,
    const uint n_points,
    const uint n_cells,
    const float3 domain_lo,
    const float3 domain_hi,
    const Point_t probe)
{
    const uint id = get_global_id(0);
    if (id < n_points) {
        /* Probe hash key */
        float3 probe_upos = (probe.pos - domain_lo) / (domain_hi - domain_lo);
        uint3 probe_cell = (uint3) (
            (float) n_cells * probe_upos.x,
            (float) n_cells * probe_upos.y,
            (float) n_cells * probe_upos.z);
        uint probe_key = hash(probe_cell);

        /* Point hash key */
        float3 point_upos = (points[id].pos - domain_lo) / (domain_hi - domain_lo);
        uint3 point_cell = (uint3) (
            (float) n_cells * point_upos.x,
            (float) n_cells * point_upos.y,
            (float) n_cells * point_upos.z);
        uint point_key = hash(point_cell);

        /* Color the point by its distance to the probe */
        if (point_key == probe_key) {
            points[id].col = point_upos;
        } else {
            points[id].col = kGray;
        }
    }
}

/** --------------------------------------------------------------------------
 * update_points
 */
__kernel void update_points(
    const __global Point_t *points,
    const uint n_points,
    const float3 domain_lo,
    const float3 domain_hi)
{
    const uint id = get_global_id(0);
    if (id < n_points) {
    }
}

/** --------------------------------------------------------------------------
 * update_vertex
 */
__kernel void update_vertex(
    __global float *vertex,
    const __global Point_t *points,
    const uint n_points)
{
    const uint id = get_global_id(0);
    if (id < n_points) {
        vertex[6*id + 0] = points[id].pos.x;
        vertex[6*id + 1] = points[id].pos.y;
        vertex[6*id + 2] = points[id].pos.z;
        vertex[6*id + 3] = points[id].col.x;
        vertex[6*id + 4] = points[id].col.y;
        vertex[6*id + 5] = points[id].col.z;
    }
}
