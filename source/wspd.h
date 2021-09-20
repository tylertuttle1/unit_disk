struct WellSeparatedPair
{
    int a, b;
};

struct WSPD
{
    int point_count;
    v2 *points;

    f32 separation_ratio;

    int pair_count;
    int max_pair_count;
    WellSeparatedPair *pairs;
};

WSPD build_wspd(v2 *points, int point_count, CentroidTree *tree, int root, f32 separation_ratio);
