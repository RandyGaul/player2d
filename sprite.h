#ifndef SPRITE_H
#define SPRITE_H

#include <math.h>

#include <glad/glad.h>
#include <cute_gl.h>
#include <cute_spritebatch.h>
#include <cute_png.h>

typedef struct
{
	float x, y;
	float u, v;
} vertex_t;

#define SPRITE_VERTS_MAX (1024 * 10)
int sprite_verts_count;
vertex_t sprite_verts[SPRITE_VERTS_MAX];
int sprite_batch_draw_call_count;
gl_draw_call_t sprite_batch_draw_calls[256];

// example of a game sprite
typedef struct
{
	SPRITEBATCH_U64 image_id;
	int depth;
	float x, y;
	float sx, sy;
	float c, s;
} sprite_t;

cp_image_t get_image(SPRITEBATCH_U64 image_id)
{
	if (image_id < 140) {
		return images[image_id];
	} else {
		static int loaded_non_tile_images = 0;
		static cp_image_t hero_idle[4];

		if (!loaded_non_tile_images) {
			loaded_non_tile_images = 1;
			hero_idle[0] = cp_load_png("art/player/idle/anim1.png");
			hero_idle[1] = cp_load_png("art/player/idle/anim2.png");
			hero_idle[2] = cp_load_png("art/player/idle/anim3.png");
			hero_idle[3] = cp_load_png("art/player/idle/anim4.png");
			for (int i = 0; i < 4; ++i) assert(hero_idle[i].pix);
		}

		if (image_id < 144) {
			return hero_idle[image_id - 140];
		}
	}

	cp_image_t img = { 0 };
	return img;
}

sprite_t make_sprite(SPRITEBATCH_U64 image_id, float x, float y, float scale, float angle_radians, int depth)
{
	cp_image_t img = get_image(image_id);
	sprite_t s;
	s.image_id = image_id;
	s.depth = depth;
	s.x = x;
	s.y = y;
	s.sx = (float)img.w * scale;
	s.sy = (float)img.h * scale;
	s.c = cosf(angle_radians);
	s.s = sinf(angle_radians);
	return s;
}

// callbacks for cute_spritebatch.h
void batch_report(spritebatch_sprite_t* sprites, int count, int texture_w, int texture_h, void* udata)
{
	(void)udata;
	(void)texture_w;
	(void)texture_h;
	//printf("begin batch\n");
	//for (int i = 0; i < count; ++i) printf("\t%llu\n", sprites[i].texture_id);
	//printf("end batch\n");

	// build the draw call
	gl_draw_call_t call;
	call.r = &sprite_renderable;
	call.textures[0] = (uint32_t)sprites[0].texture_id;
	call.texture_count = 1;

	// set texture uniform in shader
	gl_send_texture(call.r->program, "u_sprite_texture", 0);

	// NOTE:
	// perform any additional sorting here

	// build vertex buffer of quads from all sprite transforms
	call.verts = sprite_verts + sprite_verts_count;
	call.vert_count = count * 6;
	sprite_verts_count += call.vert_count;
	assert(sprite_verts_count < SPRITE_VERTS_MAX);

	vertex_t* verts = (vertex_t*)call.verts;
	for (int i = 0; i < count; ++i)
	{
		spritebatch_sprite_t* s = sprites + i;

		v2 quad[] = {
			{ -0.5f,  0.5f },
			{  0.5f,  0.5f },
			{  0.5f, -0.5f },
			{ -0.5f, -0.5f },
		};

		for (int j = 0; j < 4; ++j)
		{
			float x = quad[j].x;
			float y = quad[j].y;

			// scale sprite about origin
			x *= s->sx;
			y *= s->sy;

			// rotate sprite about origin
			float x0 = s->c * x - s->s * y;
			float y0 = s->s * x + s->c * y;
			x = x0;
			y = y0;

			// translate sprite into the world
			x += s->x;
			y += s->y;

			quad[j].x = x;
			quad[j].y = y;
		}

		// output transformed quad into CPU buffer
		vertex_t* out_verts = verts + i * 6;

		out_verts[0].x = quad[0].x;
		out_verts[0].y = quad[0].y;
		out_verts[0].u = s->minx;
		out_verts[0].v = s->maxy;

		out_verts[1].x = quad[3].x;
		out_verts[1].y = quad[3].y;
		out_verts[1].u = s->minx;
		out_verts[1].v = s->miny;

		out_verts[2].x = quad[1].x;
		out_verts[2].y = quad[1].y;
		out_verts[2].u = s->maxx;
		out_verts[2].v = s->maxy;

		out_verts[3].x = quad[1].x;
		out_verts[3].y = quad[1].y;
		out_verts[3].u = s->maxx;
		out_verts[3].v = s->maxy;

		out_verts[4].x = quad[3].x;
		out_verts[4].y = quad[3].y;
		out_verts[4].u = s->minx;
		out_verts[4].v = s->miny;

		out_verts[5].x = quad[2].x;
		out_verts[5].y = quad[2].y;
		out_verts[5].u = s->maxx;
		out_verts[5].v = s->miny;
	}

	// Buffer up the draw calls on a local stack. These will be submit to cute_gl in *reverse*
	// stack order to properly preserve sprite render ordering.
	assert(sprite_batch_draw_call_count < 256);
	sprite_batch_draw_calls[sprite_batch_draw_call_count++] = call;
}

void get_pixels(SPRITEBATCH_U64 image_id, void* buffer, int bytes_to_fill, void* udata)
{
	(void)udata;
	cp_image_t img = get_image(image_id);
	memcpy(buffer, img.pix, bytes_to_fill);
}

SPRITEBATCH_U64 generate_texture_handle(void* pixels, int w, int h, void* udata)
{
	(void)udata;
	GLuint location;
	glGenTextures(1, &location);
	glBindTexture(GL_TEXTURE_2D, location);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	return (SPRITEBATCH_U64)location;
}

void destroy_texture_handle(SPRITEBATCH_U64 texture_id, void* udata)
{
	(void)udata;
	GLuint id = (GLuint)texture_id;
	glDeleteTextures(1, &id);
}

spritebatch_config_t get_demo_config()
{
	spritebatch_config_t config;
	spritebatch_set_default_config(&config);
	config.pixel_stride = sizeof(cp_pixel_t);
	config.atlas_width_in_pixels = 1024;
	config.atlas_height_in_pixels = 1024;
	config.atlas_use_border_pixels = 0;
	config.ticks_to_decay_texture = 3;
	config.lonely_buffer_count_till_flush = 1;
	config.ratio_to_decay_atlas = 0.5f;
	config.ratio_to_merge_atlases = 0.25f;
	config.allocator_context = 0;
	return config;
}

void setup_spritebatch()
{
	// setup cute_spritebatch configuration
	// this configuration is specialized to test out the demo. don't use these settings
	// in your own project. Instead, start with `spritebatch_set_default_config`.
	spritebatch_config_t config = get_demo_config();
	//spritebatch_set_default_config(&config); // turn default config off to test out demo

	// assign the 4 callbacks
	config.batch_callback = batch_report;                       // report batches of sprites from `spritebatch_flush`
	config.get_pixels_callback = get_pixels;                    // used to retrieve image pixels from `spritebatch_flush` and `spritebatch_defrag`
	config.generate_texture_callback = generate_texture_handle; // used to generate a texture handle from `spritebatch_flush` and `spritebatch_defrag`
	config.delete_texture_callback = destroy_texture_handle;    // used to destroy a texture handle from `spritebatch_defrag`

	// initialize cute_spritebatch
	spritebatch_init(&sb, &config, NULL);
}

void push_sprite(sprite_t sp)
{
	cp_image_t img = get_image(sp.image_id);
	spritebatch_push(&sb, sp.image_id, img.w, img.h, sp.x, sp.y, sp.sx, sp.sy, sp.c, sp.s, (SPRITEBATCH_U64)sp.depth);
}

void flush_sprite_draw_calls()
{
	for (int i = sprite_batch_draw_call_count - 1; i >= 0; --i)
		gl_push_draw_call(gfx, sprite_batch_draw_calls[i]);
	sprite_batch_draw_call_count = 0;
	sprite_verts_count = 0;
}

#endif // SPRITE_H
