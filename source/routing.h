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

// @TODO: don't do this
#define MAX_GLOBAL_TABLE_SIZE 512

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
global DijkstraResult *global_dijkstra_results;

PREORDER_TRAVERSE(global_table_callback_internal)
{
    if (tree->nodes[node].point != -1) {
        umm *pointers = (umm *) data;
        int numpairs  = (int) pointers[0];
        int numleaves = (int) pointers[1];
        int i         = (int) pointers[2];
        WellSeparatedPair *pairs = (WellSeparatedPair *) pointers[3];
        RoutingTable *routing_tables = (RoutingTable *) pointers[4];
        Graph *unit_disk_graph = (Graph *) pointers[5];

        int alpha = numpairs % numleaves;
        int f = numpairs / numleaves;
        int c = (numpairs + numleaves - 1) / numleaves;
        int threshold = alpha * c;

        int x = (i < threshold) ? c : f;

        for (int j = i; j < min(i+x, numpairs); ++j) {
            int point = tree->nodes[node].point;

            size_t table_index = routing_tables[point].global_table_size;
            routing_tables[point].global_table[table_index].pair = pairs[j];

            int start = point;
            int end = tree->nodes[pairs[j].b].representative;
            DijkstraResult dijkstra_result = global_dijkstra_results[start];
            int midpoint = find_midpoint(&dijkstra_result, start, end);
            routing_tables[point].global_table[table_index].midpoint = midpoint;

            routing_tables[point].global_table_size += 1;
        }
    }
}

PREORDER_TRAVERSE(global_table_callback)
{
    // @TODO: under construction
    // we have to do a traversal for each internal node of the tree,
    // so we can't use the iterative traversal that i implemented
    // since it changes the structure of the tree. probably best to
    // switch to the regular recursive traversal, at least for the outer one.

    umm *pointers = (umm *) data;
    RoutingTable *routing_tables = (RoutingTable *) pointers[0];
    WSPD *wspd = (WSPD *) pointers[1];
    Graph *unit_disk_graph = (Graph *) pointers[2];
    WellSeparatedPair *pairs = (WellSeparatedPair *) allocate_memory(sizeof(*pairs) * MAX_GLOBAL_TABLE_SIZE);
    int pair_count = 0;
    for (int i = 0; i < wspd->pair_count; ++i) {
        if (wspd->pairs[i].a == node || wspd->pairs[i].b == node) {
            pairs[pair_count].a = node;
            pairs[pair_count].b = (wspd->pairs[i].a == node) ? wspd->pairs[i].a : wspd->pairs[i].b;

            ++pair_count;
            ++total_pair_count;
        }
    }

    if (pair_count > 0) {
        u64 *callback_data = (u64 *) allocate_memory(sizeof(*callback_data) * 5);
        callback_data[0] = pair_count;
        callback_data[1] = tree->nodes[node].size;
        callback_data[2] = 0;
        callback_data[3] = (umm) pairs;
        callback_data[4] = (umm) routing_tables;
        callback_data[5] = (umm) unit_disk_graph;

        preorder_traverse_iterative(tree, node, global_table_callback_internal, callback_data);

        free_memory(callback_data);
    }
}

internal RoutingTable *
build_routing_tables(Graph *unit_disk_graph, Graph *mst, CentroidTree *centroid_tree, WSPD *wspd)
{
    RoutingTable *routing_tables = (RoutingTable *) allocate_memory(sizeof(*routing_tables) * mst->vertex_count);
    preorder_traverse_iterative(centroid_tree, 0, local_table_callback, routing_tables);

    // @TODO: make sure the local tables are sorted in clockwise order.

    for (size_t i = 0; i < mst->vertex_count; ++i) {
        routing_tables[i].global_table = (GlobalTableEntry *) allocate_memory(sizeof(GlobalTableEntry) * MAX_GLOBAL_TABLE_SIZE);
        routing_tables[i].global_table_size = 0;
    }

    umm *global_data = (umm *) allocate_memory(sizeof(*global_data) * 2);
    global_data[0] = (umm) routing_tables;
    global_data[1] = (umm) wspd;
    global_data[2] = (umm) unit_disk_graph;
    preorder_traverse_recursive(centroid_tree, 0, global_table_callback, global_data);
    free_memory(global_data);

    printf("found %d pairs, wanted %d pairs\n", total_pair_count / 2, wspd->pair_count);
    return routing_tables;
}


internal bool
pair_contains(WSPD *wspd, WellSeparatedPair pair, int point)
{
    return false;
}

internal int
get_level(CentroidTree *centroid_tree, int point)
{
    return 0;
}

internal void
find_routing_path(v2 *points, int point_count, WSPD *wspd, RoutingTable *tables, int start, int destination, CentroidTree *centroid_tree)
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

        if (!routing_header.initialized) {
            routing_header.initialized = current_point;
            routing_header.current_level = get_level(centroid_tree, current_point);
        }

        // @TODO: find the next neighbour of `current_point` in the local table

        // if (found_r) {
        //     current_point = r;
        // } else {
        //     ++routing_header.level;
        // }
    }
}

#define ROUTING_H
#endif
