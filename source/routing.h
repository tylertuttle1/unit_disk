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
    int level;

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
    int start_site;
    int last_site;
    int current_target;
    int target_stack[4096];
    int sp;
};

PREORDER_TRAVERSE(local_table_callback);
PREORDER_TRAVERSE(global_table_callback);
PREORDER_TRAVERSE(global_table_callback_internal);
PREORDER_TRAVERSE(pair_contains_callback);

internal RoutingTable *build_routing_tables(Graph *unit_disk_graph, Graph *mst, CentroidTree *centroid_tree, WSPD *wspd);
internal bool pair_contains(CentroidTree *centroid_tree, WellSeparatedPair pair, int point);
internal int forward_message(int site, Header *header, RoutingTable *routing_tables, Graph *unit_disk_graph, CentroidTree *centroid_tree, v2 *points);
internal void find_routing_path(v2 *points,
                  size_t point_count,
                  Graph *unit_disk_graph,
                  Graph *mst,
                  CentroidTree *centroid_tree,
                  WSPD *wspd,
                  RoutingTable *routing_tables,
                  int start,
                  int target);

global int total_pair_count = 0;
global DijkstraResult *global_dijkstra_results;
