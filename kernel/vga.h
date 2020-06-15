#ifndef VGA_H_
#define VGA_H_

enum vga_mode_t {
	VGA_320x200x4,
	VGA_640x480x16,
	VGA_720x480x16,
	VGA_320x200x256,
	VGA_320x200x256_x,
};

void vga_setmode(const enum vga_mode_t);

void vga_write_regs(unsigned char *regs);
unsigned get_fb_seg(void);
void draw_pixel(unsigned x, unsigned y, unsigned c);
void draw_box(unsigned x1, unsigned y1, unsigned x2, unsigned y2, unsigned c);
void draw_circle(unsigned xpos, unsigned ypos, unsigned r, unsigned c);

void demo_graphics();

#endif
