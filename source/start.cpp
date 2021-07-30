#if defined(_MSC_VER)
#define COMPILER_MSVC 1
#define COMPILER_GCC  0
#elif defined(__GNUC__)
#define COMPILER_MSVC 0
#define COMPILER_GCC  1
#else
#error Compiler not supported!
#endif

#include <assert.h>
#include <immintrin.h>
#include <stdio.h>
#include <string.h>

#include "basic.h"
#include "vector.h"
#include "graph.h"
#include "image.h"
#include "rng.h"
#include "centroid_tree.h"
#include "wspd.h"

u64
hash_pair(s32 ix, s32 iy)
{
    assert((-16777216 <= ix) && (ix <= 16777216));
    assert((-16777216 <= iy) && (iy <= 16777216));

    // (16777216 + ix)*61 + (16777216 + iy)
    u64 result = 1040187392 + 61*ix + iy;

    return result;
}

#define POINT_COUNT 250
global f32 scale = 10.0f;
global f32 scale_x = scale;
global f32 scale_y = scale;
// global char ttf_buffer[1<<25];
global u32 colours[] = {
    0xffff0000,
    0xff0000ff,
    0xff00ff00,
    0xffffff00,
    0xff00ffff,
    0xffff00ff,
    0xffbada55,
    0xffff7f7f,
    0xff7fff7f,
    0xff7f7fff,
    0xffffffff,
    0xffffffff,
    0xffffffff,
    0xffffffff,
    0xffffffff,
    0xffffffff,
    0xffffffff,
};


void
draw_edges(Image *image, Graph *graph, u32 colour)
{
    for (int i = 0; i < graph->vertex_count; ++i) {
        v2 point = graph->points[i];
        int degree = graph->degrees[i];
        int base_index = graph->offsets[i];

        for (int offset = 0; offset < degree; ++offset) {
            int neighbour_id = graph->adjacency_list[base_index + offset];

            if (i < neighbour_id) {
                v2 neighbour = graph->points[neighbour_id];

                int x0 = image->width * point.x / scale_x;
                int y0 = image->height * (scale_y - point.y) / scale_y;

                int x1 = image->width * neighbour.x / scale_x;
                int y1 = image->height * (scale_y - neighbour.y) / scale_y;

                // draw_line(&image, x0, y0, x1, y1, colours[i % arraycount(colours)]);
                draw_line(image, x0, y0, x1, y1, colour);
            }
        }
    }
}

void
draw_points(Image *image, v2 *points, int point_count, u32 colour)
{
    for (int i = 0; i < point_count; ++i) {
        f32 x = points[i].x;
        f32 y = points[i].y;

        int min_x = (int)(image->width*x/scale_x - 2.5f);
        int max_x = (int)(image->width*x/scale_x + 2.5f);
        int min_y = (int)(image->height*(scale_y - y)/scale_y - 2.5f);
        int max_y = (int)(image->height*(scale_y - y)/scale_y + 2.5f);

        u32 colour = bgra32_pack(1.0f, 0.0f, 0.0f, 1.0f);

        draw_rectangle(image, min_x, min_y, max_x, max_y, colour);
    }
}


global int pad = 30;
global int hgap = 20;
global int vgap = 20;

void
calculate_widths(CentroidTree *tree, int node_index, int *widths)
{
    CentroidTreeNode *node = &tree->nodes[node_index];

    if (node->point == -1) {
        // this is an internal node
        calculate_widths(tree, node->left, widths);
        calculate_widths(tree, node->right, widths);
        widths[node_index] = widths[node->left] + widths[node->right] + hgap;
    } else {
        widths[node_index] = 0;
    }
}

void
draw_centroid_tree_internal(CentroidTree *tree, int node_index, Image *image, int *widths, int offset_x, int offset_y, int depth)
{
    CentroidTreeNode *node = &tree->nodes[node_index];

    int x = offset_x + widths[node_index] / 2;
    int y = offset_y;

    if (node->point == -1) {
        int left_width = widths[node->left];

        int left_x = offset_x + widths[node->left] / 2;
        int left_y = offset_y + vgap;

        draw_line(image, x, y, left_x, left_y, 0xffffffff);

        int right_x = offset_x + left_width + hgap + widths[node->right] / 2;
        int right_y = offset_y + vgap;

        draw_line(image, x, y, right_x, right_y, 0xffffffff);

        draw_centroid_tree_internal(tree, node->left, image, widths, offset_x, offset_y + vgap, depth + 1);
        draw_centroid_tree_internal(tree, node->right, image, widths, offset_x + left_width + hgap, offset_y + vgap, depth + 1);
    }

    int min_x = x - 2;
    int max_x = x + 2;
    int min_y = y - 2;
    int max_y = y + 2;

    u32 colour = (node->point == -1) ? colours[depth] : 0xffff0000;
    draw_rectangle(image, min_x, min_y, max_x, max_y, colour);
}

Image
draw_centroid_tree(CentroidTree *tree)
{
    // first we need to compute the required image size
    // the height is height(tree)*vgap + 2*pad
    // the width is width(root) + 2*pad,
    // where width(root) = width(root.left) + width(root.right) + hgap

    int height_needed = vgap*tree->height + 2*pad;
    int *widths = (int *) allocate_memory(tree->node_count * sizeof(int));

    calculate_widths(tree, 0, widths);

    int width_needed = widths[0] + 2*pad;

    Image result = create_image(width_needed, height_needed);

    draw_centroid_tree_internal(tree, 0, &result, widths, pad, pad, 0);

    return result;
}
 
int
start(void)
{
    RNG rng;
    seed_rng(&rng, 123123, 123123);

    v2 points[POINT_COUNT] = {};
    for (int i = 0; i < POINT_COUNT; ++i) {
        points[i].x = scale_x * random_f32(&rng);
        points[i].y = scale_y * random_f32(&rng);
    }

    Image image1 = create_image(512, 512);
    Image image2 = create_image(512, 512);

    u64 freq = get_clock_frequency();
    u64 A = get_clock();

    Graph graph = build_unit_disk_graph(points, POINT_COUNT);

    u64 B = get_clock();

    Graph mst = compute_emst(points, POINT_COUNT);

    u64 C = get_clock();

    draw_edges(&image1, &graph, 0xff333333);
    draw_edges(&image1, &mst, 0xff00ff00);
    draw_points(&image1, points, POINT_COUNT, 0xff0000ff);

    u64 D = get_clock();

    CentroidTree centroid_tree = build_centroid_tree(&mst);
    for (int i = 0; i < centroid_tree.node_count; ++i) {
        assert(centroid_tree.nodes[i].left >= 0);
        assert(centroid_tree.nodes[i].right >= 0);
    }
    draw_edges(&image2, &graph, 0xff333333);

    int queue[2*POINT_COUNT - 1] = {};
    int index;
    int colour_index = 0;
    int head = 0;
    int tail = 0;
    int one_past_last_index_on_current_level = 0;

    queue[tail++] = 0;
    one_past_last_index_on_current_level = tail;

    while (head < tail) {
        index = queue[head++];

        if (head > one_past_last_index_on_current_level) {
            one_past_last_index_on_current_level = tail;
            ++colour_index;
        }

        if (colour_index == 3) {
            int y = 3;
        }

        CentroidTreeNode node = centroid_tree.nodes[index];

        v2 p = node.edge.p;
        v2 q = node.edge.q;

        int x0 = image2.width * p.x / scale_x;
        int y0 = (image2.height - 1) * (scale_y - p.y) / scale_y;

        int x1 = image2.width * q.x / scale_x;
        int y1 = (image2.height - 1) * (scale_y - q.y) / scale_y;

        draw_line(&image2, x0, y0, x1, y1, colours[colour_index % arraycount(colours)]);

        // @NOTE: if node is internal
        if (node.point == -1) {
            queue[tail++] = node.left;
            queue[tail++] = node.right;
        }
    }

    draw_points(&image2, points, POINT_COUNT, 0xff0000ff);

    for (int i = 0; i < centroid_tree.node_count; ++i) {
        assert(centroid_tree.nodes[i].left >= 0);
        assert(centroid_tree.nodes[i].right >= 0);
    }
    Image tree_image = draw_centroid_tree(&centroid_tree);
    save_image(tree_image, "tree.bmp");

    WSPD wspd = build_wspd(points, POINT_COUNT, &centroid_tree, 0, 0.05);
    // for (int i = 0; i < wspd.pair_count; ++i) {
    //     WellSeparatedPair pair = wspd.pairs[i];
    //     printf("[%d] (%d, %d)\n", i, pair.a, pair.b);
    // }

    u64 E = get_clock();

    printf("time to build unit disk graph: %f ms\n", milliseconds(A, B, freq));
    printf("time to build minimum spanning tree: %f ms\n", milliseconds(B, C, freq));
    printf("time to build centroid tree: %f ms\n", milliseconds(D, E, freq));
    printf("time to draw graph: %f ms\n", milliseconds(C, D, freq));

    save_image(image1, "out_mst.bmp");
    save_image(image2, "out.bmp");

    Image font_bitmap = load_image("fontfont.bmp");
    Image test_output = create_image(512, 512);

    int x = 100;
    int y = 100;

    int font_cols = 16;
    int glyph_width  = 8;
    int glyph_height = 8;
    char *s = "Testy ^:)";

    draw_string(&test_output, &font_bitmap, s, x, y, font_cols, glyph_width, glyph_height);
    save_image(test_output, "fontfonttest.bmp");

#if 0
    // @TODO: make this into a function that we can call to draw text.

    FILE *fp = fopen("vollkorn.ttf", "rb");

    fseek(fp, 0, SEEK_END);
    s32 filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    u8 *filedata = (u8 *) malloc(filesize);
    fread(filedata, filesize, 1, fp);

    fclose(fp);

    Image font_bitmap;
    font_bitmap.width = 256;
    font_bitmap.height = 256;
    font_bitmap.stride = font_bitmap.width;
    font_bitmap.size_in_bytes = font_bitmap.width * font_bitmap.height;
    font_bitmap.pixels = malloc(font_bitmap.size_in_bytes);

    stbtt_bakedchar characters[95];
    int result = stbtt_BakeFontBitmap(filedata, 0, 32, (u8 *) font_bitmap.pixels, font_bitmap.width, font_bitmap.height, ' ', 95, characters);

    stbtt_fontinfo font_info;
    int tes = stbtt_InitFont(&font_info, filedata, 0);
    printf("%d\n", tes);

    Image output_image = create_image(256, 256);

    u8 *source = (u8 *) font_bitmap.pixels;
    u8 *dest_row = (u8 *) output_image.pixels;

    for (int i = 0; i < 256; ++i) {
        u32 *dest_pixel = (u32 *) dest_row;
        for (int j = 0; j < 256; ++j) {
            u8 value = *source++;

            u32 dest_red   = 0xff & (*dest_pixel >> 16);
            u32 dest_green = 0xff & (*dest_pixel >> 8);
            u32 dest_blue  = 0xff & (*dest_pixel);

            u32 red   = ((255 - value)*dest_red + value*255) >> 8;
            u32 green = ((255 - value)*dest_green + value*255) >> 8;
            u32 blue  = ((255 - value)*dest_blue + value*255) >> 8;

            *dest_pixel++ = 0xff000000 | (value << 16) | (value << 8) | value;
        }

        dest_row += output_image.stride;
    }

    char *output_string = "Hello, world!";

    f32 fx = 10;
    int y = 200;


    char *at = output_string;
    while (*at) {
        int x = round_to_s32(fx);
        // draw_rectangle(&output_image, x-1, y-1, x+1, y+1, 0xff00ffff);

        stbtt_bakedchar bc = characters[*at - ' '];

        // printf("character: '%c'\n", *at);
        // printf("%d %d\n", x, y);
        // printf("%d %d %d %d\n", bc.x0, bc.y0, bc.x1, bc.y1);
        // printf("%d %d\n", bc.x1 - bc.x0, bc.y1 - bc.y0);
        // printf("%.2f %.2f %.2f\n\n", bc.xoff, bc.yoff, bc.xadvance);

        int sx = x + bc.xoff;
        int sy = y + bc.yoff;

        u8 *source_row = (u8 *) font_bitmap.pixels + font_bitmap.width * (int) bc.y0 + (int) bc.x0;
        u8 *dest_row = (u8 *) output_image.pixels + output_image.stride * (int) sy + 4 * (int) sx;
        for (int i = 0; i < (int)(bc.y1 - bc.y0); ++i) {
            u32 *dest_pixel = (u32 *) dest_row;
            u8 *source_pixel = source_row;
            for (int j = 0; j < (int)(bc.x1 - bc.x0); ++j) {
                u8 value = *source_pixel++;

                u8 dest_red   = 0xff & (*dest_pixel >> 16);
                u8 dest_green = 0xff & (*dest_pixel >> 8);
                u8 dest_blue  = 0xff & (*dest_pixel);

                u8 red   = ((255 - value)*dest_red + value*255) / 256;
                u8 green = ((255 - value)*dest_green + value*255) / 256;
                u8 blue  = ((255 - value)*dest_blue + value*255) / 256;

                *dest_pixel++ = 0xff000000 | (red << 16) | (green << 8) | blue;
            }

            source_row += font_bitmap.width;
            dest_row += output_image.stride;
        }

        // if (*at != ' ') {
        //     draw_rectangle_outline(&output_image, sx, sy, sx+bc.x1 - bc.x0, sy+bc.y1 - bc.y0, 0xffff0000);
        // }

        if (*(at + 1)) {
            f32 scale = stbtt_ScaleForPixelHeight(&font_info, 32);
            fx += scale * stbtt_GetCodepointKernAdvance(&font_info, *at, *(at + 1));
        }
        fx += bc.xadvance;
        ++at;
    }

    save_image(output_image, "text_test.bmp");
#endif

    return 0;
}
