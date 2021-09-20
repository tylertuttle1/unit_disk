struct Graph
{
    int vertex_count;
    int edge_count;

    v2 *points;

    int *offsets;
    int *degrees;
    int *adjacency_list;
};


struct UnionFind
{
    int *parents;
    int element_count;
};

// @TODO: we only use the points here for drawing code now, so we can
// get rid of them eventually.
struct Edge
{
    u32 pi, qi;
    v2 p, q;
    f32 distance_squared;
};

struct DijkstraResult
{
    int *prev;
    f32 *dist;
};

Graph create_graph(int vertex_count);
void destroy_graph(Graph *graph);
bool points_are_adjacent(Graph *graph, int p, int q);
void remove_edge(Graph *graph, int p, int q);
Graph build_unit_disk_graph(v2 *points, int point_count);

void initialize(UnionFind *uf);
int find(UnionFind *uf, int value);
void merge(UnionFind *uf, int a, int b);

Graph compute_emst(v2 *points, int point_count);

#define DFS_CALLBACK(name) void name(Graph *graph, void *data, int vertex_id, int parent_id)
typedef DFS_CALLBACK(DFSCallback);
void depth_first_search_internal(Graph *graph, int start_vertex, int *discovered, DFSCallback *preorder_callback, DFSCallback *postorder_callback, void *callback_data);
void depth_first_search(Graph *graph, int start_vertex, DFSCallback *preorder_callback, DFSCallback *postorder_callback, void *callback_data);

DijkstraResult dijkstra(Graph *graph, int source);
internal int find_midpoint(DijkstraResult *dijkstra_result, int source, int u);
