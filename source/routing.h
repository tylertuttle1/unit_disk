#if !defined(ROUTING_H)

#include "basic.h"
#include "wspd.h"

struct LocalTableEntry
{
    int neighbour_id;
    int level;
};

struct GlobalTableEntry
{
    WSPair pair;
    int midpoint;
};

struct RoutingTable
{
    LocalTableEntry local_table[6];
    size_t local_table_size;

    GlobalTableEntry *global_table;
    size_t global_table_size;
};

struct Header
{
    int initialized;
    int current_level;
    // @TODO: how big does this actually need to be? dynamically allocate
    // once we have a real bound on it?
    int target_stack[4096];
    int sp;
};

void
find_routing_path(v2 *points, int point_count, WSPD *wspd, RoutingTable *tables, int start, int destination)
{
    Header routing_header = {0};

    int current_point = start;

    while (current_point != destination) {
        RoutingTable routing_table = tables[current_point];

        // @NOTE: global routing
        for (int i = 0; i < routing_table.global_table_size; ++i) {
            GlobalTableEntry entry = routing_table.global_table[i];
            if (pair_contains(wspd, entry.pair, destination)) {
                if (distance_squared(points[current_point], points[destination]) <= 1.0f) {
                    current_point = destination;
                } else {
                    routing_header.target_stack[routing_header.sp++] = destination;
                    destination = entry.midpoint;
                }
            }
        }

        if (!header.initialized) {
            header.initialized = current_point;
            header.level = get_level(centroid_tree, current_point);
        }

        // @TODO: find the next neighbour of `current_point` in the local table

        if (found_r) {
            current_point = r;
        } else {
            ++header.level;
        }
    }
}

#define ROUTING_H
#endif
