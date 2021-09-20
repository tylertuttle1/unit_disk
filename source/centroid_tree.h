struct CentroidTreeNode
{
    int left;
    int right;

    int height;
    int depth;

    // if this node is not a leaf, this is
    // the edge that this node represents
    Edge edge;

    // if this node is a leaf, this is the index
    // of the point that it is storing. otherwise
    // it's set to -1
    int point;

    // these are for constructing the wspd
    int size;
    int representative;
};

struct CentroidTree
{
    // underlying tree?
    Graph *graph;

    int height;

    size_t max_node_count;
    int node_count;
    CentroidTreeNode *nodes;
};

#define PREORDER_TRAVERSE(name) void name(CentroidTree *tree, void *data, int node)
typedef PREORDER_TRAVERSE(TraverseCallback);

bool is_single_vertex(Graph *tree, int root);
internal void preorder_traverse_recursive(CentroidTree *tree, int t, TraverseCallback *callback, void *callback_data);
internal void postorder_traverse_recursive(CentroidTree *tree, int t, TraverseCallback *callback, void *callback_data);
internal void preorder_traverse_iterative(CentroidTree *tree, int t, TraverseCallback *callback, void *callback_data);

DFS_CALLBACK(compute_sizes);
Edge find_centroid_edge(Graph *tree, int root);
internal void set_depths(CentroidTree *tree, int node, int current_depth);
int build_centroid_tree_internal(CentroidTree *tree, int start_vertex);
CentroidTree build_centroid_tree(Graph *mst);
internal int get_level(CentroidTree *centroid_tree, int point);
