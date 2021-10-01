PREORDER_TRAVERSE(local_table_callback)
{
    if (tree->nodes[node].point == -1) {
        RoutingTable *routing_tables = (RoutingTable *) data;

        int pi = tree->nodes[node].edge.pi;
        int qi = tree->nodes[node].edge.qi;

        routing_tables[pi].local_table[routing_tables[pi].local_table_size].neighbour_id = qi;
        routing_tables[pi].local_table[routing_tables[pi].local_table_size].level = tree->nodes[node].depth;
        ++routing_tables[pi].local_table_size;

        routing_tables[qi].local_table[routing_tables[qi].local_table_size].neighbour_id = pi;
        routing_tables[qi].local_table[routing_tables[qi].local_table_size].level = tree->nodes[node].depth;
        ++routing_tables[qi].local_table_size;
    }
}

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
        int point = tree->nodes[node].point;

        // printf("\tadding %d pairs to %d (%d):\t", MIN(i+x, numpairs) - i, tree->nodes[node].point, routing_tables[point].global_table_size);

        for (int j = i; j < MIN(i+x, numpairs); ++j) {
            size_t table_index = routing_tables[point].global_table_size;
            routing_tables[point].global_table[table_index].pair = pairs[j];

            int start = point;
            int end = tree->nodes[pairs[j].b].representative;
            DijkstraResult dijkstra_result = global_dijkstra_results[start];
            int midpoint = find_midpoint(&dijkstra_result, start, end);
            routing_tables[point].global_table[table_index].midpoint = midpoint;

            routing_tables[point].global_table_size += 1;
            WellSeparatedPair pair = routing_tables[point].global_table[table_index].pair;
            // printf("(%d, %d) ", pair.a, pair.b);
        }

        // printf("new size of global table is %d\n", routing_tables[point].global_table_size);
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
            pairs[pair_count].b = (wspd->pairs[i].a == node) ? wspd->pairs[i].b : wspd->pairs[i].a;
            assert(pairs[pair_count].a != pairs[pair_count].b);

            ++pair_count;
            ++total_pair_count;
        }
    }

    if (pair_count > 0) {
        u64 callback_data[6];
        callback_data[0] = pair_count;
        callback_data[1] = tree->nodes[node].size;
        callback_data[2] = 0;
        callback_data[3] = (umm) pairs;
        callback_data[4] = (umm) routing_tables;
        callback_data[5] = (umm) unit_disk_graph;

        // printf("distributing %d pairs in subtree of %d (%d leaves)\n", pair_count, node, tree->nodes[node].size);
        preorder_traverse_iterative(tree, node, global_table_callback_internal, callback_data);
    }
}

internal RoutingTable *
build_routing_tables(Graph *unit_disk_graph, Graph *mst, CentroidTree *centroid_tree, WSPD *wspd, v2 *points)
{
    RoutingTable *routing_tables = (RoutingTable *) allocate_memory(sizeof(*routing_tables) * mst->vertex_count);
    preorder_traverse_iterative(centroid_tree, 0, local_table_callback, routing_tables);

    // @TODO: make sure the local tables are sorted in clockwise order.

    for (size_t i = 0; i < mst->vertex_count; ++i) {
        routing_tables[i].global_table = (GlobalTableEntry *) allocate_memory(sizeof(GlobalTableEntry) * MAX_GLOBAL_TABLE_SIZE);
        routing_tables[i].global_table_size = 0;

        routing_tables[i].level = get_level(centroid_tree, i);

        LocalTableEntry *local_table = routing_tables[i].local_table;
        for (size_t j = 1; j < routing_tables[i].local_table_size; ++j) {
            v2 c = points[i];

            size_t k = j;
            while (k > 0) {
                v2 a = points[local_table[k].neighbour_id];
                v2 b = points[local_table[k-1].neighbour_id];

                if (less_than(a, b, c)) {
                    break;
                }

                myswap(routing_tables[i].local_table[k-1], routing_tables[i].local_table[k]);
                --k;
            }
        }
    }

    umm global_data[3];
    global_data[0] = (umm) routing_tables;
    global_data[1] = (umm) wspd;
    global_data[2] = (umm) unit_disk_graph;
    preorder_traverse_recursive(centroid_tree, 0, global_table_callback, global_data);

    // printf("found %d pairs, wanted %d pairs\n", total_pair_count / 2, wspd->pair_count);
    return routing_tables;
}


PREORDER_TRAVERSE(pair_contains_callback)
{
    int *data_ints = (int *) data;
    int target = data_ints[0];
    int *found = data_ints + 1;
    CentroidTreeNode tree_node = tree->nodes[node];

    if (tree_node.point == target) {
        *found = 1;
    }
}

internal bool
pair_contains(CentroidTree *centroid_tree, WellSeparatedPair pair, int point)
{
    int callback_data[2];
    callback_data[0] = point;
    callback_data[1] = 0;
    preorder_traverse_recursive(centroid_tree, pair.a, pair_contains_callback, callback_data);
    if (callback_data[1]) {
        return true;
    } else {
        preorder_traverse_recursive(centroid_tree, pair.b, pair_contains_callback, callback_data);
        if (callback_data[1]) {
            return true;
        }
    }

    return false;
}

#if 0
#define debug_log(...) printf(__VA_ARGS__)
#else
#define debug_log(...)
#endif

internal int
forward_message(int site, Header *header, RoutingTable *routing_tables, Graph *unit_disk_graph, CentroidTree *centroid_tree, v2 *points)
{
    int target = header->current_target;

    persistent int testy = 100;
    // if (testy++ < 30)
        debug_log("current site: %d, target: %d\t", site, target);

    if (site == target) {
        if (header->sp) {
            --header->sp;
            header->current_target = header->target_stack[header->sp];
            debug_log("popping %d from the stack(%d) and making it the new target\n", header->current_target, header->sp);

            return site;
        } else {
            return site;
        }
    } else {
        RoutingTable table = routing_tables[site];

        GlobalTableEntry entry;
        bool found = false;
        for (int i = 0; i < table.global_table_size; ++i) {
            entry = table.global_table[i];
            if (pair_contains(centroid_tree, entry.pair, header->current_target)) {
                // debug_log("found!\n");
                found = true;
                break;
            }
        }

        if (found) {
            // debug_log("global routing step\n");
            debug_log("pair (%d, %d) contains %d\t", entry.pair.a, entry.pair.b, target);
            header->start_site = -1;
            if (points_are_adjacent(unit_disk_graph, site, target)) {
                debug_log("there is an edge from %d to %d, moving there\n", site, target);
                header->last_site = -1;
                return target;
            } else {
                // @NOTE: push header->current_target on to the stack
                debug_log("pushing %d onto the stack, setting %d to new target\n", header->current_target, entry.midpoint);

                header->target_stack[header->sp] = header->current_target;
                ++header->sp;
                header->current_target = entry.midpoint;
                if (testy < 30) {
                    // debug_log("setting new target to %d\n", entry.midpoint);
                }
                return site;
            }
        } else {
            debug_log("local routing step\n");
            if (header->start_site == -1) {
                header->start_site = site;
                header->current_level = table.level;
            }

            // @TODO: make this the next neighbour of site in clockwise order
            // @IMPORTANT: big brain time -- do they even have to be in clockwise order?!?!?!?

            // @NOTE: header->last_site is not guaranteed to be one of our neighbours
            // in the minimum spanning tree
#if 1
            if (header->last_site >= 0) {
                // @NOTE we need a way to check if we're back at the start
                // but first, let's just find the next neighbour

                debug_log("we came from %d\n", header->last_site);
                debug_log("we are at %d\n", site);

                int last_site_index = 0;
                while (header->last_site != table.local_table[last_site_index].neighbour_id
                    && last_site_index < table.local_table_size)
                {
                    debug_log("our neighbour in the table is %d\n", table.local_table[last_site_index].neighbour_id);
                    ++last_site_index;
                }

                if (last_site_index == table.local_table_size) {
                    // we must have just done a global routing step
                    // find 
                }

                // @NOTE: we always came from a neighbour in the EMST
                // assert(last_site_index < table.local_table_size);

                int next_site_index = (last_site_index + 1) % table.local_table_size;
                while (header->current_level > table.local_table[next_site_index].level
                       && next_site_index != last_site_index)
                {
                    next_site_index = (next_site_index + 1) % table.local_table_size;
                    // debug_log("whee\n");
                }

                if (site == header->start_site) {
                    persistent int test = 0;
                    int first_next_site_index = 0;
                    while (table.local_table[first_next_site_index].level < header->current_level && first_next_site_index < table.local_table_size) {
                        ++first_next_site_index;
                    }

                    // if (test++ < 20) {
                    //     debug_log("right back where we started (%d, %d, %d)\n", header->current_level, next_site_index, first_next_site_index);
                    // }

                    if (next_site_index == first_next_site_index) {
                        // @NOTE: we've completed the euler tour
                        --header->current_level;
                        header->last_site = -1;
                        return site;
                    }
                }

                header->last_site = site;
                return table.local_table[next_site_index].neighbour_id;
#if 0
                int first_next_site_index = -1;
                if (site == header->start_site) {
                    first_next_site_index = 0;
                    while (table.local_table[first_next_site_index].level < header->current_level) {
                        first_next_site_index = (first_next_site_index + 1) % table.local_table_size;
                    }
                }

                int last_site_index = 0;
                while (header->last_site != table.local_table[last_site_index].neighbour_id) {
                    ++last_site_index;
                }

                int next_site_index = (last_site_index + 1) % table.local_table_size;
                while (table.local_table[next_site_index].level < header->current_level) {
                    next_site_index = (next_site_index + 1) % table.local_table_size;
                }

                if (first_next_site_index == next_site_index) {
                    --header->current_level;
                    return site;
                } else {
                    header->last_site = site;
                    return table.local_table[next_site_index].neighbour_id;
                }
#endif
            } else {
                int next_site_index = 0;
                while (table.local_table[next_site_index].level < header->current_level && next_site_index < table.local_table_size) {
                    ++next_site_index;
                }

                if (next_site_index == table.local_table_size) {
                    --header->current_level;
                    return site;
                }

                header->last_site = site;
                return table.local_table[next_site_index].neighbour_id;
            }
#else
            int r = -1;

            v2 current_point = points[site];
            v2 current_min;
            int min_id;
            for (min_id = 0; min_id < table.local_table_size; ++min_id) {
                if (table.local_table[min_id].level >= header->current_level) {
                    current_min = points[table.local_table[min_id].neighbour_id];
                    break;
                }
            }
            for (int i = min_id+1; i < table.local_table_size; ++i) {
                v2 test_point = points[table.local_table[i].neighbour_id];
                if ((table.local_table[i].level >= header->current_level) && less_than(test_point, current_min, current_point)) {
                    current_min = test_point;
                    min_id = table.local_table[i].neighbour_id;
                }
            }

            if (header->last_site >= 0) {
                v2 current_min2;
                int min_id2 = -1;

                for (min_id2 = 0; min_id2 < table.local_table_size; ++min_id2) {
                    if (table.local_table[min_id2].level >= header->current_level
                        && less_than(points[header->last_site], points[table.local_table[min_id2].neighbour_id], current_point))
                    {
                        current_min2 = points[table.local_table[min_id2].neighbour_id];
                        break;
                    }
                }
                for (int i = 0; i < table.local_table_size; ++i) {
                    v2 test_point = points[table.local_table[i].neighbour_id];
                    if ((table.local_table[i].level >= header->current_level)
                        && less_than(points[header->last_site], test_point, current_point)
                        && less_than(test_point, current_min2, current_point))
                    {
                        current_min2 = test_point;
                        min_id2 = table.local_table[i].neighbour_id;
                    }
                }

                if (min_id != min_id2) {
                    r = min_id2;
                }
            } else {
                r = min_id;
            }

            if (r == -1) {
                --header->current_level;
                return site;
            } else {
                header->last_site = site;
                return table.local_table[r].neighbour_id;
            }
#endif

        }
    }
}

internal f32
find_routing_path(v2 *points,
                  size_t point_count,
                  Graph *unit_disk_graph,
                  Graph *mst,
                  CentroidTree *centroid_tree,
                  WSPD *wspd,
                  RoutingTable *routing_tables,
                  int start,
                  int target)
{
    Header header = {0};

    f32 total_distance = 0.0f;

    header.initialized = 1;
    header.current_level = -1;
    header.start_site = -1;
    header.last_site = -1;
    header.current_target = target;
    header.sp = 0;

    int last_site = -1;
    int current_site = start;
    while (current_site != target) {
    // for (int i = 0; i < 20; ++i) {
        if (current_site != last_site) {
            printf("%d -> ", current_site);
            if (last_site >= 0) {
                total_distance += distance(points[last_site], points[current_site]);
                assert(points_are_adjacent(unit_disk_graph, current_site, last_site));
            }
            last_site = current_site;
        }
        current_site = forward_message(current_site, &header, routing_tables, unit_disk_graph, centroid_tree, points);
    }
    printf("%d\n", current_site);

    total_distance += distance(points[last_site], points[current_site]);
    return total_distance;
}
