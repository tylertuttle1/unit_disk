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

#define USE_SSE2
#include "sse_mathfun.h"

u64
hash_pair(s32 ix, s32 iy)
{
    assert((-16777216 <= ix) && (ix <= 16777216));
    assert((-16777216 <= iy) && (iy <= 16777216));

    // (16777216 + ix)*61 + (16777216 + iy)
    u64 result = 1040187392 + 61*ix + iy;

    return result;
}

f32
floor_f32(f32 value)
{
    f32 result = _mm_cvtss_f32(_mm_floor_ps(_mm_set_ss(value)));
    return result;
}

f32
ceil_f32(f32 value)
{
    f32 result = _mm_cvtss_f32(_mm_ceil_ps(_mm_set_ss(value)));
    return result;
}

f32
sqrt_f32(f32 value)
{
    f32 result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(value)));
    return result;
}

float
sin_f32(float value)
{
    float result = _mm_cvtss_f32(sin_ps(_mm_set_ss(value)));
    return result;
}

#define POINT_COUNT 20
global f32 scale = 1.5f;
global f32 scale_x = scale;
global f32 scale_y = scale;
// global char ttf_buffer[1<<25];

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

int
start(void)
{
    RNG rng;
    seed_rng(&rng, 69, 421);

    v2 points[POINT_COUNT] = {};
    for (int i = 0; i < POINT_COUNT; ++i) {
        points[i].x = scale_x * random_f32(&rng);
        points[i].y = scale_y * random_f32(&rng);
    }

    Image image = create_image(512, 512);

    u64 freq = get_clock_frequency();
    u64 A = get_clock();

    Graph graph = build_unit_disk_graph(points, POINT_COUNT);

    u64 B = get_clock();

    Graph mst = compute_emst(points, POINT_COUNT);

    u64 C = get_clock();

    draw_edges(&image, &graph, 0xff333333);
    draw_edges(&image, &mst, 0xff00ff00);
    draw_points(&image, points, POINT_COUNT, 0xff0000ff);

    Edge centroid_edge = find_centroid_edge(&mst, 0);

    u64 D = get_clock();

    CentroidTree centroid_tree = build_centroid_tree(&mst);

    u64 E = get_clock();

    printf("time to build unit disk graph: %f ms\n", milliseconds(A, B, freq));
    printf("time to build minimum spanning tree: %f ms\n", milliseconds(B, C, freq));
    printf("time to build centroid tree: %f ms\n", milliseconds(D, E, freq));
    printf("time to draw graph: %f ms\n", milliseconds(C, D, freq));

    save_image(image, "out.bmp");

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
