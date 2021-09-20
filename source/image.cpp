
Image
create_image(int width, int height)
{
    Image result = {};

    result.width         = width;
    result.height        = height;
    result.stride        = 4 * width;
    result.size_in_bytes = result.stride * result.height;
    result.pixels        = allocate_memory(result.size_in_bytes);

    u32 *pixel = (u32 *) result.pixels;
    for (int y = 0; y < result.height; ++y) {
        for (int x = 0; x < result.width; ++x) {
            *pixel++  = 0xff000000;
        }
    }

    return result;
}


u32
bgra32_pack(v4 colour)
{
    u32 red   = round_to_u32(255.0f * colour.r);
    u32 green = round_to_u32(255.0f * colour.g);
    u32 blue  = round_to_u32(255.0f * colour.b);
    u32 alpha = round_to_u32(255.0f * colour.a);

    u32 result = (alpha << 24) | (red << 16) | (green << 8) | blue;

    return result;
}

u32
bgra32_pack(f32 r, f32 g, f32 b, f32 a)
{
    u32 red   = round_to_u32(255.0f * r);
    u32 green = round_to_u32(255.0f * g);
    u32 blue  = round_to_u32(255.0f * b);
    u32 alpha = round_to_u32(255.0f * a);

    u32 result = (alpha << 24) | (red << 16) | (green << 8) | blue;

    return result;
}

v4
bgra32_unpack(u32 colour)
{
    f32 red   = (f32)((colour >> 16) & 0xff) / 255.0f;
    f32 green = (f32)((colour >>  8) & 0xff) / 255.0f;
    f32 blue  = (f32)((colour >>  0) & 0xff) / 255.0f;
    f32 alpha = (f32)((colour >> 24) & 0xff) / 255.0f;

    v4 result = {red, blue, green, alpha};

    return result;
}

u8
u8_pack(f32 value)
{
    u8 result = round_to_u32(255.0f * value);
    return result;
}

f32
u8_unpack(u8 value)
{
    f32 result = (f32)(value) / 255.0f;
    return result;
}

Image
load_image(char const *filename)
{
    Image result = {};

    FILE *fp = fopen(filename, "rb");

    if (fp) {
        fseek(fp, 0, SEEK_END);
        s32 filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        u8 *filedata = (u8 *) allocate_memory(filesize);
        size_t bytes_read = fread(filedata, 1, filesize, fp);
        assert(bytes_read == filesize);

        fclose(fp);

        // @NOTE: this is not a fully-featured bitmap loader. right now it only supports
        // the format that paint.net outputs on my machine.
        BitmapFileHeader file_header = *(BitmapFileHeader *) filedata;
        BitmapHeader header = *(BitmapHeader *)(filedata + sizeof(BitmapFileHeader));

        int stride   = header.width * sizeof(u32);

        result.width  = header.width;
        result.height = abs(header.height);
        result.stride = stride;
        result.size_in_bytes = stride * header.height;
        result.pixels = allocate_memory(result.size_in_bytes);

        u8 *source_row = filedata + file_header.bitmap_offset + (header.height - 1)*stride;
        u8 *dest_row   = (u8 *) result.pixels;

        for (int y = 0; y < header.height; ++y) {
            u32 *source_pixel = (u32 *) source_row;
            u32 *dest_pixel   = (u32 *) dest_row;

            for (int x = 0; x < header.width; ++x) {
                u32 colour = *source_pixel++;

                u32 red_shift   = lsb(header.red_mask);
                u32 green_shift = lsb(header.green_mask);
                u32 blue_shift  = lsb(header.blue_mask);
                u32 alpha_shift = lsb(header.alpha_mask);

                u32 red   = (colour & header.red_mask) >> red_shift;
                u32 green = (colour & header.green_mask) >> green_shift;
                u32 blue  = (colour & header.blue_mask) >> blue_shift;
                u32 alpha = (colour & header.alpha_mask) >> alpha_shift;

                *dest_pixel++ = (alpha << 24) | (red << 16) | (green << 8) | blue;
            }

            source_row -= stride;
            dest_row += stride;
        }
    }

    return result;
}

void
save_image(Image image, char const *filename)
{
#define twocc(s) (u16)(((s)[0]) | ((s)[1] << 8))

    BitmapFileHeader output_file_header = {};
    output_file_header.type          = twocc("BM");
    output_file_header.size          = sizeof(BitmapFileHeader) + sizeof(BitmapHeader) + image.size_in_bytes;
    output_file_header.bitmap_offset = sizeof(BitmapFileHeader) + sizeof(BitmapHeader);

    BitmapHeader output_header = {};
    output_header.size             = sizeof(output_header);
    output_header.width            = image.width;
    output_header.height           = -image.height;
    output_header.planes           = 1;
    output_header.bits_per_pixel   = 32;
    output_header.compression_type = 3; // 0 is BI_RGB, 3 is BI_BITFIELDS
    output_header.image_size       = image.size_in_bytes;
    output_header.red_mask         = 0x00ff0000;
    output_header.green_mask       = 0x0000ff00;
    output_header.blue_mask        = 0x000000ff;
    output_header.alpha_mask       = 0xff000000;

    // @TODO: file api in the platform layer?
    FILE *fp = fopen(filename, "wb");
    if (fp) {
        fwrite(&output_file_header, sizeof(BitmapFileHeader), 1, fp);
        fwrite(&output_header, sizeof(BitmapHeader), 1, fp);
        fwrite(image.pixels, image.size_in_bytes, 1, fp);
        fclose(fp);
    } else {
        fprintf(stdout, "cannot open file %s for writing!\n", filename);
    }
}

void
blit_bitmap(Image *dest, int dest_x, int dest_y, Image *source, int source_x, int source_y, int width, int height)
{
    u8 *source_row = (u8 *) source->pixels + source_y*source->stride + source_x*4;
    u8 *dest_row   = (u8 *) dest->pixels + dest_y*dest->stride + dest_x*4;

    for (int y = 0; y < height; ++y) {
        u32 *source_pixel = (u32 *) source_row;
        u32 *dest_pixel   = (u32 *) dest_row;
        for (int x = 0; x < width; ++x) {
            // @TODO real alpha blending
            if (*source_pixel & 0xff000000) {
                *dest_pixel = *source_pixel;
            }

            ++dest_pixel;
            ++source_pixel;
        }
        source_row += source->stride;
        dest_row   += dest->stride;
    }
}

void
draw_line(Image *image, int x0, int y0, int x1, int y1, u32 colour)
{
    assert((0 <= x0) && (x0 < image->width));
    assert((0 <= x1) && (x1 < image->width));
    assert((0 <= y0) && (y0 < image->height));
    assert((0 <= y1) && (y1 < image->height));

    bool is_steep = false;
    if (abs(x0 - x1) < abs(y0 - y1)) {
        is_steep = true;
        myswap(x0, y0);
        myswap(x1, y1);
    }

    if (x0 > x1) {
        myswap(x0, x1);
        myswap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror = 2 * abs(dy);
    int error = 0;
    int y = y0;
    int inc = (y1 > y0) ? 1 : -1;

    u32 *pixel;
    int x_inc, y_inc;
    if (is_steep) {
        pixel = (u32 *)((u8 *) image->pixels + x0*image->stride + y0*4);
        x_inc = image->width;
        y_inc = inc;
    } else {
        pixel = (u32 *)((u8 *) image->pixels + y0*image->stride + x0*4);
        x_inc = 1;
        y_inc = inc*image->width;
    }

    for (int x = x0; x <= x1; ++x) {
        *pixel = colour;
        pixel += x_inc;

        error += derror;
        if (error > dx) {
            pixel += y_inc;
            error -= 2*dx;
        }
    }
}

void
draw_rectangle(Image *image, int min_x, int min_y, int max_x, int max_y, u32 colour)
{
    min_x = MAX(min_x, 0);
    max_x = MIN(max_x, image->width);
    min_y = MAX(min_y, 0);
    max_y = MIN(max_y, image->height);

    u8 *row = (u8 *) image->pixels + min_y*image->stride + min_x*4;
    for (int y = min_y; y < max_y; ++y) {
        u32 *pixel = (u32 *) row;
        for (int x = min_x; x < max_x; ++x) {
            *pixel++ = colour;
        }
        row += image->stride;
    }
}

void
draw_rectangle_outline(Image *image, int min_x, int min_y, int max_x, int max_y, u32 colour)
{
    min_x = MAX(min_x, 0);
    max_x = MIN(max_x, image->width);
    min_y = MAX(min_y, 0);
    max_y = MIN(max_y, image->height);

    draw_line(image, min_x, min_y, min_x, max_y, colour);
    draw_line(image, min_x, min_y, max_x, min_y, colour);
    draw_line(image, max_x, min_y, max_x, max_y, colour);
    draw_line(image, min_x, max_y, max_x, max_y, colour);
}

void
draw_string(Image *image, Image *font, char const *text, int x, int y, int font_cols, int glyph_width, int glyph_height)
{
    char *at = (char *) text;
    char c;
    while ((c = *at++) != 0) {
        int glyph_index = c - 32; // @NOTE: 32 is space in ascii
        assert((0 <= glyph_index) && (glyph_index <= 94));

        int glyph_x = glyph_width * (glyph_index % font_cols);
        int glyph_y = glyph_height * (glyph_index / font_cols);

        blit_bitmap(image, x, y, font, glyph_x, glyph_y, glyph_width, glyph_height);

        x += glyph_width;
    }
}
