#if !defined(CENTROID_TREE_H)
#define CENTROID_TREE_H

#include <assert.h>

#include "basic.h"
#include "platform.h"
#include "graph.h"

struct CentroidTreeNode
{
    int left;
    int right;

    int height;

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

bool
is_single_vertex(Graph *tree, int root)
{
    return tree->degrees[root] == 0;
}

DFS_CALLBACK(compute_sizes)
{
    int *sizes = (int *) data;

    int degree = graph->degrees[vertex_id];
    int offset = graph->offsets[vertex_id];

    sizes[vertex_id] = 1;
    for (int i = 0; i < degree; ++i) {
        int neighbour_id = graph->adjacency_list[offset + i];
        if (neighbour_id != parent_id) {
            sizes[vertex_id] += sizes[neighbour_id];
        }
    }
}

Edge
find_centroid_edge(Graph *tree, int root)
{
    int *sizes = (int *) allocate_memory(tree->vertex_count * sizeof(int));
    clear_array(sizes, tree->vertex_count);

    depth_first_search(tree, root, 0, compute_sizes, (void *) sizes);
    int degree = tree->degrees[root];
    int offset = tree->offsets[root];

    sizes[root] = 1;
    for (int i = 0; i < degree; ++i) {
        int neighbour_id = tree->adjacency_list[offset + i];
        sizes[root] += sizes[neighbour_id];
    }

    int total_size = sizes[root];
    int min_threshold = (total_size + 6) / 7; // ceil(total_size / 7)
    int max_threshold = 6 * total_size / 7;

    int parent_id = -1;
    int current_vertex = root;

    for (;;) {
        int degree = tree->degrees[current_vertex];
        int offset = tree->offsets[current_vertex];

        int largest_child_size = -1;
        int largest_child = -1;
        for (int i = 0; i < degree; ++i) {
            int neighbour_id = tree->adjacency_list[offset + i];
            if (neighbour_id != parent_id) {
                if ((min_threshold <= sizes[neighbour_id]) && (sizes[neighbour_id] <= max_threshold)) {
                    Edge result;

                    result.pi = current_vertex;
                    result.qi = neighbour_id;

                    result.p = tree->points[current_vertex];
                    result.q = tree->points[neighbour_id];

                    return result;
                }

                if ((largest_child_size == -1) || (sizes[neighbour_id] > largest_child_size)) {
                    largest_child = neighbour_id;
                    largest_child_size = sizes[neighbour_id];
                }
            }
        }

        // didn't find one in the threshold
        parent_id = current_vertex;
        current_vertex = largest_child;
    }

    assert(!"invalid code path!");
}

int
build_centroid_tree_internal(CentroidTree *tree, int start_vertex)
{
    int node_index = tree->node_count;
    CentroidTreeNode *result = &tree->nodes[node_index];
    assert(tree->node_count < tree->max_node_count);
    ++tree->node_count;

    if (is_single_vertex(tree->graph, start_vertex)) {
        result->left = 0;
        result->right = 0;
        result->edge = {};
        result->point = start_vertex;
        result->representative = start_vertex;
        result->size = 1;
        result->height = 0;
    } else {
        Edge centroid_edge = find_centroid_edge(tree->graph, start_vertex);
        remove_edge(tree->graph, centroid_edge.pi, centroid_edge.qi);

        result->left = build_centroid_tree_internal(tree, centroid_edge.pi);
        result->right = build_centroid_tree_internal(tree, centroid_edge.qi);
        result->edge = centroid_edge;
        result->point = -1;
        result->representative = tree->nodes[result->left].representative;
        result->size = tree->nodes[result->left].size + tree->nodes[result->right].size;
        result->height = 1 + max(tree->nodes[result->left].height, tree->nodes[result->right].height);
    }

    return node_index;
}

CentroidTree
build_centroid_tree(Graph *mst)
{
    // centroid tree is a (full) binary tree with n leaves
    // that means there is 2n - 1 total nodes.

    CentroidTree result;

    result.graph = mst;
    result.max_node_count = 2 * mst->vertex_count - 1;
    result.node_count = 0;

    size_t size = result.max_node_count * sizeof(CentroidTreeNode);
    result.nodes = (CentroidTreeNode *) allocate_memory(size);

    build_centroid_tree_internal(&result, 0);

    result.height = result.nodes[0].height;

    return result;
}

#endif
