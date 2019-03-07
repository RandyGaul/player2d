#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <stdio.h>

#include <glad/glad.h>
#include <SDL2/SDL.h>

#define CUTE_GL_IMPLEMENTATION
#include <cute_gl.h>

#define CUTE_C2_IMPLEMENTATION
#include <cute_c2.h>

#define CUTE_PNG_IMPLEMENTATION
#include <cute_png.h>

#define SPRITEBATCH_IMPLEMENTATION
#include <cute_spritebatch.h>

#include <cute_coroutine.h>
#include <cute_math2d.h>

#define EDITOR_LAYER 3

using capsule_t = c2Capsule;
inline c2v c2(v2 v) { return c2V(v.x, v.y); }
inline v2 c2(c2v v) { return v2(v.x, v.y); }
inline aabb_t c2(c2AABB box) { return make_aabb(c2(box.min), c2(box.max)); }
inline c2AABB c2(aabb_t box) { c2AABB c2_box; c2_box.min = c2(box.min); c2_box.max = c2(box.max); return c2_box; }

int application_running = 1;
SDL_Window* window;
gl_context_t* gfx;
gl_shader_t sprite_shader;
gl_renderable_t sprite_renderable;
float projection[16];

#include <debug_draw.h>
#include <map.h>
map_t map;
map_t sprite_map;

#include <player2d.h>
player2d_t player;

// There are exactly 140 different tile images.
const int image_count = 140;
cp_image_t images[image_count];
spritebatch_t sb;

#include <sprite.h>
#include <hero.h>
hero_t hero;

#include <background.h>
background_t background;

void* read_file_to_memory(const char* path, int* size)
{
	void* data = 0;
	FILE* fp = fopen(path, "rb");
	int sizeNum = 0;

	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		sizeNum = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		data = malloc(sizeNum);
		fread(data, sizeNum, 1, fp);
		fclose(fp);
	}

	if (size) *size = sizeNum;
	return data;
}

void load_tile_images()
{
	int count = 0;
	int i = 0;
	char path[1024];

	while (count < image_count)
	{
		assert(count < 140);
		sprintf(path, "art/tile%d.png", i++);
		int size;
		void* file = read_file_to_memory(path, &size);
		if (!file) continue;
		cp_image_t img = cp_load_png_mem(file, size);
		assert(img.pix);
		images[count++] = img;
		free(file);
	}
}

float calc_dt()
{
	static int first = 1;
	static double inv_freq;
	static uint64_t prev;

	uint64_t now = SDL_GetPerformanceCounter();

	if (first) {
		first = 0;
		prev = now;
		inv_freq = 1.0 / (double)SDL_GetPerformanceFrequency();
	}

	float dt = (float)((double)(now - prev) * inv_freq);
	prev = now;
	return dt;
}

void swap_buffers()
{
	SDL_GL_SwapWindow(window);
}

int mx; // mouse x
int my; // mouse y

void main_loop()
{
	static int a_is_down = 0;
	static int d_is_down = 0;
	static int w_is_down = 0;
	static int s_is_down = 0;
	static int mouse_left_is_down = 0;
	static int ctrl_is_down = 0;

	int w_is_pressed = 0;
	int s_is_pressed = 0;
	int c_is_pressed = 0;
	int space_is_pressed = 0;
	int mouse_left_was_pressed = 0;
	int mouse_right_was_pressed = 0;
	int mouse_wheel = 0;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			application_running = 0;
			break;

		case SDL_KEYDOWN:
		{
			SDL_Keycode key = event.key.keysym.sym;
			if (key == SDLK_a) a_is_down = 1;
			if (key == SDLK_d) d_is_down = 1;
			if (key == SDLK_w) w_is_down = 1;
			if (key == SDLK_s) s_is_down = 1;
			if (key == SDLK_LCTRL || key == SDLK_RCTRL) ctrl_is_down = 1;

			if (key == SDLK_w) w_is_pressed = 1;
			if (key == SDLK_s) s_is_pressed = 1;
			if (key == SDLK_c) c_is_pressed = 1;
			if (key == SDLK_SPACE) space_is_pressed = 1;
		}	break;

		case SDL_KEYUP:
		{
			SDL_Keycode key = event.key.keysym.sym;
			if (key == SDLK_a) a_is_down = 0;
			if (key == SDLK_d) d_is_down = 0;
			if (key == SDLK_w) w_is_down = 0;
			if (key == SDLK_s) s_is_down = 0;
			if (key == SDLK_LCTRL || key == SDLK_RCTRL) ctrl_is_down = 0;
		}	break;

		case SDL_MOUSEMOTION:
			mx = event.motion.x;
			my = event.motion.y;
			mx = (int)((float)mx / 2.0f) - 160;
			my = -((int)((float)my / 2.0f) - 120);
			break;

		case SDL_MOUSEBUTTONDOWN:
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT: mouse_left_is_down = 1; mouse_left_was_pressed = 1; break;
			case SDL_BUTTON_MIDDLE: break;
			case SDL_BUTTON_RIGHT: mouse_right_was_pressed = 1; break;
			}
			break;

		case SDL_MOUSEBUTTONUP:
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT: mouse_left_is_down = 0; break;
			case SDL_BUTTON_MIDDLE: break;
			case SDL_BUTTON_RIGHT: break;
			}
			break;

		case SDL_MOUSEWHEEL:
			mouse_wheel = event.wheel.y;
			break;
		}
	}

#if 0
	// single-frame debugging, press space to take a timestep of 1/20 fps
	if (!space_is_pressed) {
		return;
	}
#endif

	float dt = calc_dt();
	if (dt > (1.0f / 20.0f)) dt = 1.0f / 20.0f;

	glClear(GL_COLOR_BUFFER_BIT);

	gl_line_color(gfx, 1.0f, 1.0f, 1.0f);

	if (!player.on_ground) {
		player.vel.y += -250.0f * dt;
	}

	float player_speed = 50.0f;
	float player_jump_speed = 150.0f;
	if (a_is_down) {
		player.vel.x = -player_speed;
		hero_set_facing(&hero, FACING_LEFT);
		if (player.on_ground && hero.state != HERO_STATE_RUN) hero_set_state(&hero, HERO_STATE_RUN);
	} else if (d_is_down) {
		player.vel.x = player_speed;
		hero_set_facing(&hero, FACING_RIGHT);
		if (player.on_ground && hero.state != HERO_STATE_RUN) hero_set_state(&hero, HERO_STATE_RUN);
	} else {
		player.vel.x = 0;
		if (player.on_ground && hero.state != HERO_STATE_IDLE) hero_set_state(&hero, HERO_STATE_IDLE);
	}

#if 0
	if (w_is_down) {
		thePlayer.vel.y = player_speed;
	} else if (s_is_down) {
		thePlayer.vel.y = -player_speed;
	} else {
		thePlayer.vel.y = 0;
	}
#endif

	// pressing jump button applies upward impulse and clears
	// ground/jump flags
	if(w_is_pressed && player.can_jump)
	{
		player.vel.y = player_jump_speed;
		player.can_jump = 0;
		player.on_ground = 0;
		hero_set_state(&hero, HERO_STATE_JUMP);
	}

	// sweep player through the world across the timestep
	float t = 1;
	v2 vel = player.vel * dt;
	int max_iters = 100;
	int iters = 0;
	while (iters++ < max_iters && t)
	{
		// sweep player and find toi
		v2 n;
		v2 contact;
		float toi = player_sweep(player.capsule, &n, &contact, vel);

		// move player to toi
		player.pos += vel * toi;
		player_sync_geometry(&player);

		if (toi == 1) break;
		t *= toi;

		// chop off the velocity along the normal
		// this is the "slide along wall" function
		vel -= n * dot(vel, n);

		// ngs out of potentially colliding configurations
		player_ngs(&player);

		// Let player jump and set ground if toi ever hits a flat plane pointing up
		// while the player is falling, only if they are hit near the feet.
		int going_down = dot(player.vel, v2(0, -1)) > 0.85f;
		float threshold = player.pos.y - ((PLAYER_HEIGHT / 2.0f) - player.capsule.r * (1.0f / 4.0f));
		int hit = toi > 0 && toi < 1;
		int hit_near_feet = hit && contact.y < threshold;
		if(hit_near_feet && going_down)
		{
			player.on_ground = 1;
			player.can_jump = 1;
		}
	}
	if (iters == max_iters) printf("Failed to exhaust timestep.\n");

	float inv_dt = dt ? 1.0f / dt : 0;
	player.vel = vel * inv_dt;

	// draw player
	draw_capsule(player.capsule);
	draw_aabb(player.box);
	gl_line(gfx, player.seg_a.x, player.seg_a.y, 0, player.seg_b.x, player.seg_b.y, 0);

	// special "on the ground" state
	if (player.on_ground) {
		player.vel.y = 0;

		if (player_can_fall(&player, 1)) {
			player.on_ground = 0;
			player.can_jump = 0;
		}

		// TODO
		// Implement sloped tiles
		// Use line segment instead of capsule to walk on sloped tiles
		// Follow slope with shape-cast (to prevent "floating" off of slopes)
	}

	// debug draw map
	gl_line_color(gfx, 1.0f, 1.0f, 1.0f);
	debug_draw_map(&map);

	static int editor = 0;
	if (mouse_right_was_pressed) {
		editor = !editor;
		printf("Editor turned %s.\n", editor ? "ON" : "OFF");
		if (editor) {
			printf(
				" > Press LEFT-CLICK on the mouse to place a sprite tile.\n"
				" > Scroll the mouse wheel to cycle through the various available tiles.\n"
				" > Hit ctrl + s to save the sprite map.\n"
				" > Hit ctrl + c to copy the currently hovered tile to the mouse selection.\n"
				" > Note: The editor can not edit the collision map. This must be done by hand in a text editor.\n"
			);
		}
	}

	if (editor) {
		static int tile_selection = 0;
		if (mouse_wheel) {
			tile_selection += mouse_wheel;
			if (tile_selection < 0) tile_selection = image_count - 1;
			else if (tile_selection >= image_count) tile_selection = 0;
		}

		int tile_x;
		int tile_y;
		get_tile_xy_from_world_pos(&map, v2((float)mx, (float)my), &tile_x, &tile_y);
		aabb_t bounds = get_tile_bounds(&map, tile_x, tile_y);
		float snap_mx = center(bounds).x;
		float snap_my = center(bounds).y;

		if (tile_selection != ~0) {
			sprite_t selection = make_sprite(tile_selection, snap_mx, snap_my, 1.0f, 0, EDITOR_LAYER);
			push_sprite(selection);
		}

		if (ctrl_is_down && c_is_pressed) {
			tile_selection = sprite_map.tiles[tile_y * sprite_map.w + tile_x];
			printf("Copied tile from map to mouse.\n");
		}

		if (ctrl_is_down && s_is_pressed) {
			save_map(&sprite_map, "map_sprites.txt");
			printf("Map saved.\n");
		}

		if (mouse_left_was_pressed) {
			sprite_map.tiles[tile_y * sprite_map.w + tile_x] = tile_selection;
		}
	}

	draw_map(&sprite_map);

	// TODO
	// Implement sloped tiles

	// TODO
	// Finalize map data

	background_draw(&background);

	// TODO
	// Add some moveable crates

	// Hero's animation controller
	hero_update(&hero, dt);
	hero_draw(&hero, player.pos);

	// Run cute_spritebatch to find sprite batches.
	// This is the most basic usage of cute_psritebatch, one defrag, tick and flush per game loop.
	// It is also possible to only use defrag once every N frames.
	// tick can also be called at different time intervals (for example, once per game update
	// but not necessarily once per screen render).
	spritebatch_defrag(&sb);
	spritebatch_tick(&sb);
	spritebatch_flush(&sb);
	flush_sprite_draw_calls();

	gl_flush(gfx, swap_buffers, 0, 640, 480);
}

void sdl_setup()
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("player2d character controller demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// set double buffer
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// immediate swaps
	SDL_GL_SetSwapInterval(0);

	SDL_GL_CreateContext(window);

	gladLoadGLLoader(SDL_GL_GetProcAddress);
	int major, minor;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
	printf("SDL says running on OpenGL version %d.%d\n", major, minor);
	printf("Glad says OpenGL version : %d.%d\n", GLVersion.major, GLVersion.minor);
	printf("OpenGL says : GL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
}

void cute_gl_setup()
{
	int clear_bits = GL_COLOR_BUFFER_BIT;
	int settings_bits = 0;
	gfx = gl_make_ctx(32, clear_bits, settings_bits);

#define SHADER_STR(x) "#version 330\n" #x

	const char* vs = SHADER_STR(
		uniform mat4 u_mvp;

		in vec2 in_pos;
		in vec2 in_uv;

		out vec2 v_uv;

		void main( )
		{
			v_uv = in_uv;
			gl_Position = u_mvp * vec4(in_pos, 0, 1);
		}
	);

	const char* ps = SHADER_STR(
		precision mediump float;
	
		uniform sampler2D u_sprite_texture;

		in vec2 v_uv;
		out vec4 out_col;

		void main()
		{
			out_col = texture(u_sprite_texture, v_uv);
		}
	);

	gl_vertex_data_t vd;
	gl_make_vertex_data(&vd, 1024 * 1024, GL_TRIANGLES, sizeof(vertex_t), GL_DYNAMIC_DRAW);
	gl_add_attribute(&vd, "in_pos", 2, CUTE_GL_FLOAT, CUTE_GL_OFFSET_OF(vertex_t, x));
	gl_add_attribute(&vd, "in_uv", 2, CUTE_GL_FLOAT, CUTE_GL_OFFSET_OF(vertex_t, u));

	gl_make_renderable(&sprite_renderable, &vd);
	gl_load_shader(&sprite_shader, vs, ps);
	gl_set_shader(&sprite_renderable, &sprite_shader);
	
	gl_ortho_2d((float)640 / 2.0f, (float)480 / 2.0f, 0, 0, projection);
	glViewport(0, 0, 640, 480);

	gl_send_matrix(&sprite_shader, "u_mvp", projection);
	gl_line_mvp(gfx, projection);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char** argv)
{
	sdl_setup();
	cute_gl_setup();
	load_map(&map, "map.txt");
	load_map(&sprite_map, "map_sprites.txt");
	load_tile_images();
	setup_spritebatch();
	hero_init(&hero);
	background_init(&background);

	printf("Press RIGHT-CLICK to turn ON/OFF the editor (starts OFF by default).\n");

	player.capsule.r = PLAYER_HALF_WIDTH;
	player.pos = v2(36.3215637f, 37.36820793f);

	while (application_running)
		main_loop();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

#define MAP_IMPLEMENTATION
#include <map.h>

#define HERO_IMPLEMENTATION
#include <hero.h>

#define BACKGROUND_IMPLEMENTATION
#include <background.h>

#include <glad/glad.c>
