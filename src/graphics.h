#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"
#include "vec3.h"
#include "port.h"
#include "math.h"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 200

enum COLOR {
    BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHT_GRAY, DARK_GRAY,
    LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN, LIGHT_RED, LIGHT_MAGENTA,
    YELLOW, WHITE
};

typedef struct Point {
    short x, y;
} Point;
typedef struct Rect {
    struct Point tl, br;
} Rect;

static void enter_13h_graphics_mode() {
    asm volatile(
        "mov   $0x0013, %%ax\n"
        "int   $0x10\n"
        "mov   $0xA000, %%ax\n"
        "mov   %%ax, %%es\n"
        : /* no outputs */
        : /* no inputs */
        : "ax"
    );
}

static void enter_text_mode() {
    asm volatile(
        "mov   $0x0003, %%ax\n"
        "int   $0x10\n"
        "mov   $0xA000, %%dx\n"
        "mov   %%dx, %%es\n"
        : /* no outputs */
        : /* no inputs */
        : "ax", "dx"
    );
}

static u16 DEPTH_BUFFER[SCREEN_WIDTH * SCREEN_HEIGHT] __attribute__((section(".framebuffer")));
static u8 FRAME_BUFFER[SCREEN_WIDTH * SCREEN_HEIGHT] __attribute__((section(".framebuffer")));

static void draw_pixel(volatile struct Point p, u8 color) {
    if(p.x >= 0 && p.x < SCREEN_WIDTH && p.y >= 0 && p.y < SCREEN_HEIGHT)
        // asm volatile(
        //     "imul  $320, %%bx\n"
        //     "add   %%ax, %%bx\n"
        //     "mov   %%cl, %%es:(%%bx)\n"
        //     : /* no outputs */
        //     : "a"(p.x), "b"(p.y), "c"(color)
        //     : "dx"
        // );
        FRAME_BUFFER[p.x + p.y * SCREEN_WIDTH] = color;
}

static void draw_pixel_with_depth(volatile struct Point p, u16 depth, u8 color) {
    if(p.x >= 0 && p.x < SCREEN_WIDTH && p.y >= 0 && p.y < SCREEN_HEIGHT) {
        u16 realDepth = 65535 / max(depth, 1);
        if(realDepth > DEPTH_BUFFER[p.x + p.y * SCREEN_WIDTH]) {
            DEPTH_BUFFER[p.x + p.y * SCREEN_WIDTH] = realDepth;
            // asm volatile(
            //     "imul  $320, %%bx\n"
            //     "add   %%ax, %%bx\n"
            //     "mov   %%cl, %%es:(%%bx)\n"
            //     : /* no outputs */
            //     : "a"(p.x), "b"(p.y), "c"(color)
            //     : "dx"
            // );
            FRAME_BUFFER[p.x + p.y * SCREEN_WIDTH] = color;
        }
    }
}

static void clear_screen(char color) {
    asm volatile(
        "mov   %%al, %%ah\n"
        "mov   $0, %%di\n"
        "push  %%ax\n"
        "shl   $16, %%eax\n"
        "pop   %%ax\n"
        "mov   $16000, %%cx\n"
        "rep\n"
        "stosl\n"
        : /* no outputs */
        : "a"(color)
        : "cx", "di"
    );
    
    for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) FRAME_BUFFER[i] = 0;
    for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) DEPTH_BUFFER[i] = 0;
}

static void push_framebuffer() {
    volatile u16 segment, offset;
    asm volatile (
        "mov %%cs, %0\n"
        : "=r" (segment)
    );
    asm volatile (
        "lea (%1), %%ax\n"
        "mov %%ax, %0\n"
        : "=r" (offset)
        : "r" (FRAME_BUFFER)
        : "%ax"
    );

    // Calculate the physical address
    volatile u32 physical_address = ((u32)segment << 4) + offset;

    asm volatile(
        // Move from 0x0000:FRAME_BUFFER to 0xA000:0x0000
        "push  %%ds\n"
        "push  %%es\n"

        // FRAME_BUFFER (in bx) -> ds
        "mov   %%bx, %%ds\n"
        // 0 -> esi
        "mov   $0x0, %%esi\n"
        // 0 -> edi
        "mov   $0, %%edi\n"
        // 0xA000 -> es
        "mov   $0xA000, %%ax\n"
        "mov   %%ax, %%es\n"

        "mov   $16000, %%cx\n" // Run this 16000 times - 64000 bytes (the framebuffer size) / 4 bytes per dword (32 bits)
        // rsi:ds to rdi:es
        "rep movsd\n" // Move dword from address (R|E)SI to (R|E)DI. On a 16-bit processor, this means it moves 4 bytes at a time.

        "pop %%es\n"
        "pop %%ds\n"
        : /* no outputs */
        : "bx"(0x101835) // WTF?? Found through trial and error. Hackiest thing ever, but I just needed to make the game run
        : "ax", "cx", "di", "si", "memory"
    );
    
    // TODO: replace with rep stosl
    for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) FRAME_BUFFER[i] = 0;
    for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) DEPTH_BUFFER[i] = 0;
}

static int abs(int x) {
    return x < 0 ? -x : x;
}

static void draw_line(Point a, Point b, u8 color) {
    i16 dx = abs(b.x - a.x), sx = a.x < b.x ? 1 : -1;
    i16 dy = abs(b.y - a.y), sy = a.y < b.y ? 1 : -1;
    i16 err = (dx > dy ? dx : -dy) / 2, e2;
    for(;;) {
        draw_pixel(a, color);
        if(a.x == b.x && a.y == b.y)
            break;
        e2 = err;
        if(e2 > -dx) {
            err -= dy;
            a.x += sx;
        }
        if(e2 < dy) {
            err += dx;
            a.y += sy;
        }
    }
}

// static inline i16 fast_sqrt(i16 v) {
//   i16 g = 0x80, c = 0x80;
//   for(;;) {
//     if(g*g > v) g ^= c;
//     c >>= 1;
//     if(c == 0) return g;
//     g |= c;
//   }
// }
// static inline i16 fast_len(i16 x, i16 y) {
//     return fast_sqrt(x * x + y * y);
// }

#define TRIANGLE_DRAW_FP_SCALE 9
static void draw_filled_triangle_with_depth(Vec3 a, Vec3 b, Vec3 c, u8 color) {
    // If the triangle doesn't intersect with the screen at all, don't draw it
    i16 xMax = max(a.x, max(b.x, c.x));
    i16 xMin = min(a.x, min(b.x, c.x));
    i16 yMax = max(a.y, max(b.y, c.y));
    i16 yMin = min(a.y, min(b.y, c.y));
    if(xMin >= SCREEN_WIDTH || xMax < 0 || yMin >= SCREEN_HEIGHT || yMax < 0) return;

    if(a.y > b.y) { Vec3 t = a; a = b; b = t; }
    if(a.y > c.y) { Vec3 t = a; a = c; c = t; }
    if(b.y > c.y) { Vec3 t = b; b = c; c = t; }
    
    i16 total_height = c.y - a.y;
    for(i16 i = 0; i < total_height; i++) {
        i16 y = a.y + i;
        if(y < 0 || y >= SCREEN_HEIGHT) continue;

        bool second_half = i > b.y - a.y || b.y == a.y;
        i16 segment_height = second_half ? c.y - b.y : b.y - a.y;
        if(segment_height == 0) continue;
        i16 alpha = ((i << TRIANGLE_DRAW_FP_SCALE) / total_height);
        i16 beta = ((i << TRIANGLE_DRAW_FP_SCALE) - ((second_half ? b.y - a.y : 0) << TRIANGLE_DRAW_FP_SCALE)) / segment_height;
        
        Vec3 A = {
            .x = a.x + (((c.x - a.x) * alpha) >> TRIANGLE_DRAW_FP_SCALE),
            .y = a.y + (((c.y - a.y) * alpha) >> TRIANGLE_DRAW_FP_SCALE),
            .z = a.z + (((c.z - a.z) * alpha) >> TRIANGLE_DRAW_FP_SCALE)
        };
        Vec3 B = {
            .x = second_half ? b.x + (((c.x - b.x) * beta) >> TRIANGLE_DRAW_FP_SCALE) : a.x + (((b.x - a.x) * beta) >> TRIANGLE_DRAW_FP_SCALE),
            .y = second_half ? b.y + (((c.y - b.y) * beta) >> TRIANGLE_DRAW_FP_SCALE) : a.y + (((b.y - a.y) * beta) >> TRIANGLE_DRAW_FP_SCALE),
            .z = second_half ? b.z + (((c.z - b.z) * beta) >> TRIANGLE_DRAW_FP_SCALE) : a.z + (((b.z - a.z) * beta) >> TRIANGLE_DRAW_FP_SCALE)
        };
        
        if(A.x > B.x) { Vec3 t = A; A = B; B = t; }
        i16 xDifference = B.x - A.x;
        if(xDifference == 0) xDifference = 1;
        for(i16 j = A.x; j <= B.x; j++) draw_pixel_with_depth((Point){ j, y }, A.z + (B.z - A.z) * (j - A.x) / xDifference, color);
    }
}

typedef struct PaletteColor {
    /// 0-63 each
    u8 r, g, b;
} PaletteColor;

static PaletteColor read_palette(u8 index) {
    outportb(0x3C7, index);
    return (PaletteColor){
        .r = inportb(0x3C9),
        .g = inportb(0x3C9),
        .b = inportb(0x3C9)
    };
}

static void write_palette(u8 index, PaletteColor color) {
    outportb(0x3C8, index);
    outportb(0x3C9, color.r);
    outportb(0x3C9, color.g);
    outportb(0x3C9, color.b);
}

static void wait_for_vsync() {
    asm volatile(
        "mov   $0x03DA, %%dx\n"
        "current%=:"
        "in    %%dx, %%al\n"
        "and   $0x8, %%al\n"
        "jnz   current%=\n"
        "restart%=:"
        "in    %%dx, %%al\n"
        "and   $0x8, %%al\n"
        "jz    restart%=\n"
        : /* no outputs */
        : /* no inputs */
        : "al", "dx"
    );
}

/// https://gist.github.com/t-mat/80af1caf3329f93ef993ebaa079e69d1
static const unsigned char BITMAP_CHARS_8x7[][8] = {
    { 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000 }, // 32 (0x20) : ' '
    { 0b00111000, 0b00111000, 0b00111000, 0b00110000, 0b00110000, 0b00000000, 0b00110000 }, // 33 (0x21) : '!'
    { 0b01101100, 0b01101100, 0b11011000, 0b00000000, 0b00000000, 0b00000000, 0b00000000 }, // 34 (0x22) : '"'
    { 0b01101100, 0b11111110, 0b01101100, 0b01101100, 0b01101100, 0b11111110, 0b01101100 }, // 35 (0x23) : '#'
    { 0b00010000, 0b01111110, 0b11010000, 0b01111100, 0b00010110, 0b11111100, 0b00010000 }, // 36 (0x24) : '$'
    { 0b01100010, 0b10100100, 0b11001000, 0b00010000, 0b00100110, 0b01001010, 0b10001100 }, // 37 (0x25) : '%'
    { 0b01110000, 0b11011000, 0b11011000, 0b01110000, 0b11011010, 0b11001100, 0b01111110 }, // 38 (0x26) : '&'
    { 0b00011000, 0b00011000, 0b00110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000 }, // 39 (0x27) : '''
    { 0b00011000, 0b00110000, 0b01100000, 0b01100000, 0b01100000, 0b00110000, 0b00011000 }, // 40 (0x28) : '('
    { 0b01100000, 0b00110000, 0b00011000, 0b00011000, 0b00011000, 0b00110000, 0b01100000 }, // 41 (0x29) : ')'
    { 0b00000000, 0b01101100, 0b00111000, 0b11111110, 0b00111000, 0b01101100, 0b00000000 }, // 42 (0x2a) : '*'
    { 0b00000000, 0b00110000, 0b00110000, 0b11111100, 0b00110000, 0b00110000, 0b00000000 }, // 43 (0x2b) : '+'
    { 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00110000, 0b00110000, 0b01100000 }, // 44 (0x2c) : ','
    { 0b00000000, 0b00000000, 0b00000000, 0b11111110, 0b00000000, 0b00000000, 0b00000000 }, // 45 (0x2d) : '-'
    { 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011000, 0b00011000 }, // 46 (0x2e) : '.'
    { 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000 }, // 47 (0x2f) : '/'
    { 0b00111000, 0b01001100, 0b11000110, 0b11000110, 0b11000110, 0b01100100, 0b00111000 }, // 48 (0x30) : '0'
    { 0b00110000, 0b01110000, 0b11110000, 0b00110000, 0b00110000, 0b00110000, 0b11111100 }, // 49 (0x31) : '1'
    { 0b01111100, 0b11000110, 0b00001110, 0b00111100, 0b01111000, 0b11100000, 0b11111110 }, // 50 (0x32) : '2'
    { 0b01111110, 0b00001100, 0b00011000, 0b00111100, 0b00000110, 0b11000110, 0b01111100 }, // 51 (0x33) : '3'
    { 0b00011100, 0b00111100, 0b01101100, 0b11001100, 0b11111110, 0b00001100, 0b00001100 }, // 52 (0x34) : '4'
    { 0b11111100, 0b11000000, 0b11111100, 0b00000110, 0b00000110, 0b11000110, 0b01111100 }, // 53 (0x35) : '5'
    { 0b00111100, 0b01100000, 0b11000000, 0b11111100, 0b11000110, 0b11000110, 0b01111100 }, // 54 (0x36) : '6'
    { 0b11111110, 0b11000110, 0b00001100, 0b00011000, 0b00110000, 0b00110000, 0b00110000 }, // 55 (0x37) : '7'
    { 0b01111000, 0b11000100, 0b11100100, 0b01111000, 0b10011110, 0b10000110, 0b01111100 }, // 56 (0x38) : '8'
    { 0b01111100, 0b11000110, 0b11000110, 0b01111110, 0b00000110, 0b00001100, 0b01111000 }, // 57 (0x39) : '9'
    { 0b00000000, 0b00110000, 0b00110000, 0b00000000, 0b00110000, 0b00110000, 0b00000000 }, // 58 (0x3a) : ':'
    { 0b00000000, 0b00011000, 0b00011000, 0b00000000, 0b00011000, 0b00011000, 0b00110000 }, // 59 (0x3b) : ';'
    { 0b00001100, 0b00011000, 0b00110000, 0b01100000, 0b00110000, 0b00011000, 0b00001100 }, // 60 (0x3c) : '<'
    { 0b00000000, 0b00000000, 0b11111110, 0b00000000, 0b11111110, 0b00000000, 0b00000000 }, // 61 (0x3d) : '='
    { 0b01100000, 0b00110000, 0b00011000, 0b00001100, 0b00011000, 0b00110000, 0b01100000 }, // 62 (0x3e) : '>'
    { 0b01111100, 0b11000110, 0b11000110, 0b00001100, 0b00111000, 0b00000000, 0b00111000 }, // 63 (0x3f) : '?'
    { 0b01111100, 0b11000110, 0b11011110, 0b11010110, 0b11011110, 0b11000000, 0b01111100 }, // 64 (0x40) : '@'
    { 0b00111000, 0b01101100, 0b11000110, 0b11000110, 0b11111110, 0b11000110, 0b11000110 }, // 65 (0x41) : 'A'
    { 0b11111100, 0b11000110, 0b11000110, 0b11111100, 0b11000110, 0b11000110, 0b11111100 }, // 66 (0x42) : 'B'
    { 0b00111100, 0b01100110, 0b11000000, 0b11000000, 0b11000000, 0b01100110, 0b00111100 }, // 67 (0x43) : 'C'
    { 0b11111000, 0b11001100, 0b11000110, 0b11000110, 0b11000110, 0b11001100, 0b11111000 }, // 68 (0x44) : 'D'
    { 0b11111110, 0b11000000, 0b11000000, 0b11111100, 0b11000000, 0b11000000, 0b11111110 }, // 69 (0x45) : 'E'
    { 0b11111110, 0b11000000, 0b11000000, 0b11111100, 0b11000000, 0b11000000, 0b11000000 }, // 70 (0x46) : 'F'
    { 0b00111110, 0b01100000, 0b11000000, 0b11001110, 0b11000110, 0b01100110, 0b00111110 }, // 71 (0x47) : 'G'
    { 0b11000110, 0b11000110, 0b11000110, 0b11111110, 0b11000110, 0b11000110, 0b11000110 }, // 72 (0x48) : 'H'
    { 0b11111100, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b11111100 }, // 73 (0x49) : 'I'
    { 0b00000110, 0b00000110, 0b00000110, 0b00000110, 0b00000110, 0b11000110, 0b01111100 }, // 74 (0x4a) : 'J'
    { 0b11000110, 0b11001100, 0b11011000, 0b11110000, 0b11111000, 0b11011100, 0b11001110 }, // 75 (0x4b) : 'K'
    { 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11111110 }, // 76 (0x4c) : 'L'
    { 0b11000110, 0b11101110, 0b11111110, 0b11111110, 0b11010110, 0b11000110, 0b11000110 }, // 77 (0x4d) : 'M'
    { 0b11000110, 0b11100110, 0b11110110, 0b11111110, 0b11011110, 0b11001110, 0b11000110 }, // 78 (0x4e) : 'N'
    { 0b01111100, 0b11000110, 0b11000110, 0b11000110, 0b11000110, 0b11000110, 0b01111100 }, // 79 (0x4f) : 'O'
    { 0b11111100, 0b11000110, 0b11000110, 0b11000110, 0b11111100, 0b11000000, 0b11000000 }, // 80 (0x50) : 'P'
    { 0b01111100, 0b11000110, 0b11000110, 0b11000110, 0b11011110, 0b11001100, 0b01111010 }, // 81 (0x51) : 'Q'
    { 0b11111100, 0b11000110, 0b11000110, 0b11001110, 0b11111000, 0b11011100, 0b11001110 }, // 82 (0x52) : 'R'
    { 0b01111100, 0b11000110, 0b11000000, 0b01111100, 0b00000110, 0b11000110, 0b01111100 }, // 83 (0x53) : 'S'
    { 0b11111100, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000 }, // 84 (0x54) : 'T'
    { 0b11000110, 0b11000110, 0b11000110, 0b11000110, 0b11000110, 0b11000110, 0b01111100 }, // 85 (0x55) : 'U'
    { 0b11000110, 0b11000110, 0b11000110, 0b11101110, 0b01111100, 0b00111000, 0b00010000 }, // 86 (0x56) : 'V'
    { 0b11000110, 0b11000110, 0b11010110, 0b11111110, 0b11111110, 0b11101110, 0b11000110 }, // 87 (0x57) : 'W'
    { 0b11000110, 0b11101110, 0b01111100, 0b00111000, 0b01111100, 0b11101110, 0b11000110 }, // 88 (0x58) : 'X'
    { 0b11001100, 0b11001100, 0b11001100, 0b01111000, 0b00110000, 0b00110000, 0b00110000 }, // 89 (0x59) : 'Y'
    { 0b11111110, 0b00001110, 0b00011100, 0b00111000, 0b01110000, 0b11100000, 0b11111110 }, // 90 (0x5a) : 'Z'
    { 0b00111100, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00111100 }, // 91 (0x5b) : '['
    { 0b10000000, 0b01000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010 }, // 92 (0x5c) : '\'
    { 0b01111000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b01111000 }, // 93 (0x5d) : ']'
    { 0b00111000, 0b01101100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000 }, // 94 (0x5e) : '^'
    { 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111110 }, // 95 (0x5f) : '_'
    { 0b00110000, 0b00110000, 0b00011000, 0b00000000, 0b00000000, 0b00000000, 0b00000000 }, // 96 (0x60) : '`'
    { 0b00000000, 0b00000000, 0b01111100, 0b00000110, 0b01111110, 0b11000110, 0b01111110 }, // 97 (0x61) : 'a'
    { 0b11000000, 0b11000000, 0b11111100, 0b11000110, 0b11000110, 0b11000110, 0b11111100 }, // 98 (0x62) : 'b'
    { 0b00000000, 0b00000000, 0b01111110, 0b11000000, 0b11000000, 0b11000000, 0b01111110 }, // 99 (0x63) : 'c'
    { 0b00000110, 0b00000110, 0b01111110, 0b11000110, 0b11000110, 0b11000110, 0b01111110 }, // 100 (0x64) : 'd'
    { 0b00000000, 0b00000000, 0b01111100, 0b11000110, 0b11111110, 0b11000000, 0b01111100 }, // 101 (0x65) : 'e'
    { 0b00011100, 0b00110000, 0b11111100, 0b00110000, 0b00110000, 0b00110000, 0b00110000 }, // 102 (0x66) : 'f'
    { 0b00000000, 0b00000000, 0b01111110, 0b11000110, 0b01111110, 0b00000110, 0b01111100 }, // 103 (0x67) : 'g'
    { 0b11000000, 0b11000000, 0b11111100, 0b11000110, 0b11000110, 0b11000110, 0b11000110 }, // 104 (0x68) : 'h'
    { 0b00011000, 0b00000000, 0b00111000, 0b00011000, 0b00011000, 0b00011000, 0b01111110 }, // 105 (0x69) : 'i'
    { 0b00011000, 0b00000000, 0b00111000, 0b00011000, 0b00011000, 0b00011000, 0b11110000 }, // 106 (0x6a) : 'j'
    { 0b11000000, 0b11000000, 0b11001110, 0b11111100, 0b11111100, 0b11011100, 0b11001110 }, // 107 (0x6b) : 'k'
    { 0b01110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b01111100 }, // 108 (0x6c) : 'l'
    { 0b00000000, 0b00000000, 0b11111100, 0b10110110, 0b10110110, 0b10110110, 0b10110110 }, // 109 (0x6d) : 'm'
    { 0b00000000, 0b00000000, 0b11111100, 0b11000110, 0b11000110, 0b11000110, 0b11000110 }, // 110 (0x6e) : 'n'
    { 0b00000000, 0b00000000, 0b01111100, 0b11000110, 0b11000110, 0b11000110, 0b01111100 }, // 111 (0x6f) : 'o'
    { 0b00000000, 0b00000000, 0b11111100, 0b11000110, 0b11000110, 0b11111100, 0b11000000 }, // 112 (0x70) : 'p'
    { 0b00000000, 0b00000000, 0b01111110, 0b11000110, 0b11000110, 0b01111110, 0b00000110 }, // 113 (0x71) : 'q'
    { 0b00000000, 0b00000000, 0b01101110, 0b01110000, 0b01100000, 0b01100000, 0b01100000 }, // 114 (0x72) : 'r'
    { 0b00000000, 0b00000000, 0b01111110, 0b11000000, 0b01111100, 0b00000110, 0b11111100 }, // 115 (0x73) : 's'
    { 0b00110000, 0b00110000, 0b11111100, 0b00110000, 0b00110000, 0b00110000, 0b00110000 }, // 116 (0x74) : 't'
    { 0b00000000, 0b00000000, 0b11000110, 0b11000110, 0b11000110, 0b11000110, 0b01111110 }, // 117 (0x75) : 'u'
    { 0b00000000, 0b00000000, 0b11001100, 0b11001100, 0b11001100, 0b01111000, 0b00110000 }, // 118 (0x76) : 'v'
    { 0b00000000, 0b00000000, 0b10110110, 0b10110110, 0b10110110, 0b10110110, 0b01111110 }, // 119 (0x77) : 'w'
    { 0b00000000, 0b00000000, 0b11000110, 0b11111110, 0b00111000, 0b11111110, 0b11000110 }, // 120 (0x78) : 'x'
    { 0b00000000, 0b00000000, 0b11000110, 0b11000110, 0b01111110, 0b00000110, 0b11111100 }, // 121 (0x79) : 'y'
    { 0b00000000, 0b00000000, 0b11111110, 0b00011100, 0b00111000, 0b01110000, 0b11111110 }, // 122 (0x7a) : 'z'
    { 0b00111000, 0b00111000, 0b01100000, 0b01100000, 0b01100000, 0b00111000, 0b00111000 }, // 123 (0x7b) : '{'
    { 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000 }, // 124 (0x7c) : '|'
    { 0b11100000, 0b11100000, 0b00110000, 0b00110000, 0b00110000, 0b11100000, 0b11100000 }, // 125 (0x7d) : '}'
    { 0b00000000, 0b00000000, 0b01110010, 0b10111010, 0b10011100, 0b00000000, 0b00000000 }, // 126 (0x7e) : '~'
    { 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000 }, // 127 (0x7f) : ' '
};

static int graphics_text_line(int c, int row) {
    return BITMAP_CHARS_8x7[c - ' '][row];
}

static void graphics_text_print(struct Point p, u8 color, const char *message) {
    for(int y = 0; y < 7; y++) {
        for(int x = 0; message[x]; x++) {
            int c = message[x];
            int line = graphics_text_line(c, y);
            for(int i = 0; i < 8; i++) {
                if((line >> i) & 0x01) {
                    int dx = p.x + x * 8 + 8 - i;
                    int dy = p.y + y;
                    draw_pixel((struct Point){dx, dy}, color);
                }
            }
        }
    }
}

static void graphics_num_print(struct Point p, u8 color, i16 num) {
    char buffer[12];
    i16 i = 0;
    if(num == 0) {
        buffer[i++] = '0';
    } else {
        if(num < 0) {
            buffer[i++] = '-';
            num = -num;
        }
        i16 n = num;
        while(n > 0) {
            n /= 10;
            i++;
        }
        buffer[i] = '\0';
        for(i--; i >= 0; i--) {
            buffer[i] = '0' + num % 10;
            num /= 10;
        }
    }
    buffer[i] = '\0';
    graphics_text_print(p, color, buffer);
}

#endif