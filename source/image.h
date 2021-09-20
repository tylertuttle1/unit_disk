#pragma pack(push, 1)
struct BitmapFileHeader
{
    u16 type;
    u32 size;
    u16 reserved[2];
    u32 bitmap_offset;
};

struct BitmapHeader
{
    u32 size;
    s32 width;
    s32 height;
    u16 planes;
    u16 bits_per_pixel;
    u32 compression_type;
    u32 image_size;
    s32 horizontal_resolution;
    s32 vertical_resolution;
    u32 colours_used;
    u32 colours_important;
    u32 red_mask;
    u32 green_mask;
    u32 blue_mask;
    u32 alpha_mask;
    u32 colour_space;
    u32 endpoints[9];
    u32 gamma_red;
    u32 gamma_greeen;
    u32 gamma_blue;
    u32 rendering_intent;
    u32 profile_offset;
    u32 profile_size;
    u32 reserved;
};
#pragma pack(pop)

struct Image
{
    int width;
    int height;
    int bytes_per_pixel;

    // @NOTE: these are calculated from the width, height, and bytes_per_pixel
    int stride;        // width * bytes_per_pixel
    int size_in_bytes; // height * stride

    void *pixels;
};

Image create_image(int width, int height);
u32 bgra32_pack(v4 colour);
u32 bgra32_pack(f32 r, f32 g, f32 b, f32 a);
v4 bgra32_unpack(u32 colour);
u8 u8_pack(f32 value);
f32 u8_unpack(u8 value);
Image load_image(char const *filename);
void save_image(Image image, char const *filename);
void blit_bitmap(Image *dest, int dest_x, int dest_y, Image *source, int source_x, int source_y, int width, int height);
void draw_line(Image *image, int x0, int y0, int x1, int y1, u32 colour);
void draw_rectangle(Image *image, int min_x, int min_y, int max_x, int max_y, u32 colour);
void draw_rectangle_outline(Image *image, int min_x, int min_y, int max_x, int max_y, u32 colour);
void draw_string(Image *image, Image *font, char const *text, int x, int y, int font_cols, int glyph_width, int glyph_height);
