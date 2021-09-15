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
    WellSeparatedPair pair;
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

PREORDER_TRAVERSE(local_table_callback)
{
    if (tree->nodes[node].point == -1) {
        RoutingTable *routing_tables = (RoutingTable *) data;

        int pi = tree->nodes[node].edge.pi;
        int qi = tree->nodes[node].edge.qi;

        routing_tables[pi].local_table[routing_tables[pi].local_table_size].neighbour_id = qi;
        routing_tables[pi].local_table[routing_tables[pi].local_table_size].level = tree->nodes[node].height;
        ++routing_tables[pi].local_table_size;

        routing_tables[qi].local_table[routing_tables[qi].local_table_size].neighbour_id = pi;
        routing_tables[qi].local_table[routing_tables[qi].local_table_size].level = tree->nodes[node].height;
        ++routing_tables[qi].local_table_size;
    }
}

global int total_pair_count = 0;

PREORDER_TRAVERSE(global_table_callback)
{
    // @TODO: under construction
    // we have to do a traversal for each internal node of the tree,
    // so we can't use the iterative traversal that i implemented
    // since it changes the structure of the tree. probably best to
    // switch to the regular recursive traversal, at least for the outer one.

    WSPD *wspd = (WSPD *) data;
    int pair_count = 0;
    for (int i = 0; i < wspd->pair_count; ++i) {
        if (wspd->pairs[i].a == node || wspd->pairs[i].b == node) {
            ++pair_count;
            ++total_pair_count;
        }
    }

    // @TODO: traverse the subtree of node, assigning the pairs

    printf("node %d has %d pairs\n", node, pair_count);
}

internal RoutingTable *
build_routing_tables(Graph *mst, CentroidTree *centroid_tree, WSPD *wspd)
{
    RoutingTable *routing_tables = (RoutingTable *) allocate_memory(sizeof(*routing_tables) * mst->vertex_count);
    preorder_traverse(centroid_tree, local_table_callback, routing_tables);
    preorder_traverse(centroid_tree, global_table_callback, wspd);
    printf("found %d pairs, wanted %d pairs\n", total_pair_count / 2, wspd->pair_count);
    return routing_tables;
}

#if 0
internal void
construct_routing_table(size_t point, size_t node, WSPD *wspd, CentroidTree *centroid_tree)
{
    // @TODO under construction

    size_t pairs[MAX_PAIRS];
    size_t sp = 0;

    for (size_t i = 0; i < wspd->pair_count; ++i) {
        if (node == wspd->pairs[i].a || node == wspd->pairs[i].b) {
            pairs[sp++] = i;
        }
    }

    size_t M = sp;
    size_t N = centroid_tree->nodes[point].size;

    // @TODO: depth first search of subtree of node.
}

void
build_routing_tables(v2 *points, int point_count, WSPD *wspd, Graph *mst, CentroidTree *centroid_tree, RoutingTable *tables)
{
    // @NOTE: build local tables
    for (int point_index = 0; point_index < point_count; ++point_index) {
        for (int i = 0; i < mst->degrees[point_index]; ++i) {
            int neighbour = mst->adjacency_list[mst->offsets[point_index] + i];
            tables[point_index].local_table[i].neighbour_id = neighbour;

            // @TODO: find the edge {point_index, neighbour} in the centroid tree
            // tables[point_index].local_table[i].level = ???;
        }
    }

#if 0
    for (each node in centroid_tree) {
        WellSeparatedPair pair_list[4096];
        int index = 0;

        for (each pair in wspd->pairs) {
            if (pair.a == node or pair.b == node) {
                pair_list[index++] = pair;
            }
        }

        // now we have a list of every pair that contains our node
        int m = index / node.size;
        int i = 0;
        for (each point in the subtree of node) {
            // assign pair_list[m*i : m*(i+1)] to point
            for (int pair_index = m*i; pair_index < m*(i+1); ++pair_index) {
                // push pair_list[pair_index] to tables[point_index].global_table
                if (point is in pair.a) {
                    // run dijkstra on point, representative(b) and find midpoint
                } else {
                    // run dijkstra on point, representative(a) and find midpoint
                }
                store midpoint in global_table
            }
            i += 1;
        }
    }
#endif
}

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
#endif

#define ROUTING_H
#endif
