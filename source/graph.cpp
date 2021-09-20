Graph
create_graph(int vertex_count)
{
    Graph result;

    result.vertex_count = vertex_count;
    result.edge_count = 0;

    size_t block_size = vertex_count * (vertex_count + 1) * sizeof(int);
    void *block = allocate_memory(block_size);
    assert(block);

    memset(block, 0, block_size);

    result.offsets = (int *) block;
    result.degrees = (int *) block + vertex_count;
    result.adjacency_list = (int *) block + 2*vertex_count;

    for (int i = 0; i < vertex_count; ++i) {
        result.offsets[i] = vertex_count * i;
    }

    return result;
}

void
destroy_graph(Graph *graph)
{
    free_memory(graph->offsets);
    memset(graph, 0, sizeof(*graph));
}

bool
points_are_adjacent(Graph *graph, int p, int q)
{
    int p_offset = graph->offsets[p];
    int q_offset = graph->offsets[q];

    int p_degree = graph->degrees[p];
    int q_degree = graph->degrees[q];

    bool p_found = false;
    bool q_found = false;

    for (int i = 0; i < p_degree; ++i) {
        int neighbour_id = graph->adjacency_list[p_offset + i];
        if (neighbour_id == q) {
            q_found = true;
            break;
        }
    }

    for (int i = 0; i < q_degree; ++i) {
        int neighbour_id = graph->adjacency_list[q_offset + i];
        if (neighbour_id == p) {
            p_found = true;
            break;
        }
    }

    return p_found && q_found;
}

void
remove_edge(Graph *graph, int p, int q)
{
    assert(points_are_adjacent(graph, p, q));

    int p_base = graph->offsets[p];
    int q_base = graph->offsets[q];

    int p_degree = graph->degrees[p];
    int q_degree = graph->degrees[q];

    int i = 0;
    while (graph->adjacency_list[p_base + i] != q) ++i;
    while (i < p_degree - 1) {
        graph->adjacency_list[p_base + i] = graph->adjacency_list[p_base + i + 1];
        ++i;
    }
    --graph->degrees[p];

    int j = 0;
    while (graph->adjacency_list[q_base + j] != p) ++j;
    while (j < q_degree - 1) {
        graph->adjacency_list[q_base + j] = graph->adjacency_list[q_base + j + 1];
        ++j;
    }
    --graph->degrees[q];
}

Graph
build_unit_disk_graph(v2 *points, int point_count)
{
    // @NOTE: rough estimate: building a unit disk graph on 100,000 points should take
    // about 30 minutes with this naive algorithm. anything under 20,000 points will
    // finish in under 10 seconds though.

    Graph result = {};

    result.vertex_count = point_count;
    result.points = points;

    size_t block_size = point_count * (point_count + 1) * sizeof(int);
    void *block = allocate_memory(block_size);
    assert(block);

    clear_memory(block, block_size);

    result.offsets = (int *) block;
    result.degrees = (int *) block + point_count;
    result.adjacency_list = (int *) block + 2*point_count;

    int index = 0;
    int offset = 0;

    // @TODO: the adjacency list is packed tightly so in theory we don't need to allocate
    // this much memory, but that would require two passes over the points. we can also
    // use a hash table to get an algorithm that's linear in the number of edges instead
    // of always being quadratic in the number of points.
    for (int i = 0; i < point_count; ++i) {
        for (int j = 0; j < point_count; ++j) {
            if (i == j) continue;

            v2 p = points[i];
            v2 q = points[j];

            if (distance_squared(p, q) <= 1.0f) {
                result.adjacency_list[index + offset] = j;
                ++offset;
                ++result.edge_count;
            }
        }

        result.offsets[i] = index;
        result.degrees[i] = offset;
        index += offset;
        offset = 0;
    }

    // we double counted the edges.
    result.edge_count /= 2;

    return result;
}

// 
// @NOTE: euclidean minimum spanning tree
// 


void
initialize(UnionFind *uf)
{
    // @NOTE: make-set
    for (int i = 0; i < uf->element_count; ++i) {
        uf->parents[i] = i;
    }
}

int
find(UnionFind *uf, int value)
{
    int result = value;
    while (uf->parents[result] != result) {
        result = uf->parents[result];
    }
    return result;
}

void
merge(UnionFind *uf, int a, int b)
{
    a = find(uf, a);
    b = find(uf, b);

    uf->parents[a] = b;
}

Graph
compute_emst(v2 *points, int point_count)
{
    u64 freq = get_clock_frequency();

    auto comparator = [](const void *a, const void *b)
    {
        Edge ea = *(Edge *) a;
        Edge eb = *(Edge *) b;

        f32 len_a = ea.distance_squared;
        f32 len_b = eb.distance_squared;

        int result;

        if (len_a < len_b) {
            result = -1;
        } else if (len_a == len_b) {
            result = 0;
        } else {
            result = 1;
        }

        return result;
    };

    Graph result = {};

    result.vertex_count = point_count;
    result.points = points;

    result.offsets = (int *) allocate_memory(point_count * sizeof(int));
    result.degrees = (int *) allocate_memory(point_count * sizeof(int));
    result.adjacency_list = (int *) allocate_memory(6 * point_count * sizeof(int));
    for (int i = 0; i < point_count; ++i) {
        result.offsets[i] = i * 6;
    }
    clear_array(result.adjacency_list, 6 * point_count);
    clear_array(result.degrees, point_count);

    int edge_count = point_count * (point_count - 1) / 2;
    Edge *edges = (Edge *) allocate_memory(edge_count * sizeof(Edge));

    int index = 0;
    for (int i = 0; i < point_count; ++i) {
        for (int j = i + 1; j < point_count; ++j) {
            edges[index].pi = i;
            edges[index].qi = j;
            edges[index].p = points[i];
            edges[index].q = points[j];
            edges[index].distance_squared = length_squared(points[j] - points[i]);
            ++index;
        }
    }

    qsort(edges, edge_count, sizeof(Edge), comparator);

    UnionFind uf;
    uf.parents = (int *) allocate_memory(point_count * sizeof(int));
    uf.element_count = point_count;

    initialize(&uf);

    int edges_added = 0;
    for (int i = 0; (i < edge_count) && (edges_added < point_count); ++i) {
        int p_offset = result.offsets[edges[i].pi];
        int q_offset = result.offsets[edges[i].qi];

        int p_degree = result.degrees[edges[i].pi];
        int q_degree = result.degrees[edges[i].qi];

        if (find(&uf, edges[i].pi) != find(&uf, edges[i].qi)) {
            result.adjacency_list[p_offset + p_degree] = edges[i].qi;
            result.adjacency_list[q_offset + q_degree] = edges[i].pi;

            ++result.degrees[edges[i].pi];
            ++result.degrees[edges[i].qi];

            ++edges_added;

            merge(&uf, edges[i].pi, edges[i].qi);
        }
    }

    free_memory(edges);
    free_memory(uf.parents);

    return result;
}

// 
// @NOTE: depth-first search
// 


void
depth_first_search_internal(Graph *graph, int start_vertex, int *discovered, DFSCallback *preorder_callback, DFSCallback *postorder_callback, void *callback_data)
{
    discovered[start_vertex] = 1;

    int degree = graph->degrees[start_vertex];
    int base_index = graph->offsets[start_vertex];

    for (int offset = 0; offset < degree; ++offset) {
        int neighbour_id = graph->adjacency_list[base_index + offset];

        if (!discovered[neighbour_id]) {
            if (preorder_callback) preorder_callback(graph, callback_data, neighbour_id, start_vertex);
            depth_first_search_internal(graph, neighbour_id, discovered, preorder_callback, postorder_callback, callback_data);
            if (postorder_callback) postorder_callback(graph, callback_data, neighbour_id, start_vertex);
        }
    }
}

void
depth_first_search(Graph *graph, int start_vertex, DFSCallback *preorder_callback, DFSCallback *postorder_callback, void *callback_data)
{
    int *discovered = (int *) allocate_memory(sizeof(int) * graph->vertex_count);
    clear_array(discovered, graph->vertex_count);

    depth_first_search_internal(graph, start_vertex, discovered, preorder_callback, postorder_callback, callback_data);

    free_memory(discovered);
}


DijkstraResult
dijkstra(Graph *graph, int source)
{
    DijkstraResult result;

    // MINHEAP is a """mean heap""" storing all the vertices of the graph
    int heap_size = graph->vertex_count;

    size_t block_size = graph->vertex_count * (2 * sizeof(int) + sizeof(f32));
    void *block = allocate_memory(block_size);

    int *heap = (int *) block;
    int *prev = (int *) block + graph->vertex_count;
    f32 *dist = (f32 *)((int *) block + 2*graph->vertex_count);

    for (int i = 0; i < graph->vertex_count; ++i) {
        dist[i] = INFINITY;
        prev[i] = -1;
        heap[i] = i;
    }

    dist[source] = 0.0f;

    while (heap_size > 0) {
        // 1. find the vertex in Q with min dist[i]
        int min_index = 0;
        for (int i = 1; i < heap_size; ++i) {
            if (dist[heap[i]] < dist[heap[min_index]]) {
                min_index = i;
            }
        }

        // 2. remove that vertex from Q
        int v = heap[min_index];
        for (int i = min_index+1; i < heap_size; ++i) {
            heap[i-1] = heap[i];
        }
        --heap_size;

        // 3. for each neighbour of that vertex ...
        int offset = graph->offsets[v];
        int degree = graph->degrees[v];
        for (int i = 0; i < degree; ++i) {
            int neighbour = graph->adjacency_list[offset + i];

            // if neighbour is in heap
            int found = 0;
            for (int j = 0; j < heap_size; ++j) {
                if (heap[j] == neighbour) {
                    found = 1;
                    break;
                }
            }
            if (found) {
                v2 p = graph->points[v];
                v2 q = graph->points[neighbour];
                f32 test_value = dist[v] + distance(p, q);

                if (test_value < dist[neighbour]) {
                    dist[neighbour] = test_value;
                    prev[neighbour] = v;
                }
            }
        }
    }

    result.prev = prev;
    result.dist = dist;

    return result;
}

internal int
find_midpoint(DijkstraResult *dijkstra_result, int source, int u)
{
    int midpoint;
    bool midpoint_found = false;
    f32 total_distance = dijkstra_result->dist[u];
    // v2 last_p = points[u];
    while (u != source) {
        if (!midpoint_found && (dijkstra_result->dist[dijkstra_result->prev[u]] < (0.5f * total_distance))) {
            // we know that either u or prev[u] is the midpoint.
            if (dijkstra_result->dist[dijkstra_result->prev[u]] > (total_distance - dijkstra_result->dist[u])) {
                midpoint = dijkstra_result->prev[u];
            } else {
                midpoint = u;
            }

            return midpoint;
        }

        u = dijkstra_result->prev[u];
        // last_p = point;
    }

    return u;
}


#if 0
DFS_CALLBACK(my_dfs_callback)
{
    Image *image = (Image *) data;

    v2 point = graph->points[vertex_id];
    v2 parent = graph->points[parent_id];

    int x0 = image->width * point.x / scale_x;
    int y0 = image->height * (scale_y - point.y) / scale_y;
    int x1 = image->width * parent.x / scale_x;
    int y1 = image->height * (scale_y - parent.y) / scale_y;

    draw_line(image, x0, y0, x1, y1, 0xff00ff00);

    return 0;
}
#endif
