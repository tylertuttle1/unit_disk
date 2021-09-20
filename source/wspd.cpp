WSPD
build_wspd(v2 *points, int point_count, CentroidTree *tree, int root, f32 separation_ratio)
{
    WSPD result = {};

    result.point_count = point_count;
    result.points = points;
    result.separation_ratio = separation_ratio;

    // @TODO: how many pairs do we actually need?
#define MAX_PAIRS 10*65536
    result.pair_count = 0;
    result.max_pair_count = MAX_PAIRS;
    result.pairs = (WellSeparatedPair *) allocate_memory(MAX_PAIRS * sizeof(WellSeparatedPair));

    WellSeparatedPair queue[MAX_PAIRS] = {};
    int queue_head = 0;
    int queue_tail = 0;
    queue[queue_tail++] = {root, root};

    while (queue_head < queue_tail) {
        WellSeparatedPair candidate = queue[queue_head++];

        CentroidTreeNode a = tree->nodes[candidate.a];
        CentroidTreeNode b = tree->nodes[candidate.b];

        v2 rep_a = points[a.representative];
        v2 rep_b = points[b.representative];

        if (distance(rep_a, rep_b) > (separation_ratio + 2) * MAX(a.size - 1, b.size - 1)) {
            assert(result.pair_count < result.max_pair_count);
            assert(candidate.a != candidate.b);
            result.pairs[result.pair_count++] = candidate;
        } else if ((a.size > 1) || (b.size > 1)) {
            assert(queue_tail < (MAX_PAIRS - 1));
            if (a.size < b.size) {
                queue[queue_tail++] = {b.left, candidate.a};
                queue[queue_tail++] = {b.right, candidate.a};
            } else {
                queue[queue_tail++] = {a.left,  candidate.b};
                queue[queue_tail++] = {a.right, candidate.b};
            }
        }
    }

    return result;
}
