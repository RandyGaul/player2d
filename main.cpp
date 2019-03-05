#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <stdio.h>

#include <glad/glad.h>
#include <SDL2/SDL.h>

#define CUTE_GL_IMPLEMENTATION
#include <cute_gl.h>

#include <cute_c2.h>

#define CUTE_PNG_IMPLEMENTATION
#include <cute_png.h>

#include <cute_coroutine.h>
#include <cute_math2d.h>

using capsule_t = c2Capsule;
inline c2v c2(v2 v) { return c2V(v.x, v.y); }
inline v2 c2(c2v v) { return v2(v.x, v.y); }

int application_running = 1;
SDL_Window* window;
gl_context_t* gfx;
//gl_shader_t font_shader;
//gl_renderable_t font_renderable;
float projection[16];

#include <debug_draw.h>
#include <map.h>
map_t map;

#include <player2d.h>
player2d_t player;

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

void main_loop()
{
	static int a_is_down = 0;
	static int d_is_down = 0;
	static int w_is_down = 0;
	static int s_is_down = 0;

	int w_is_pressed = 0;
	int space_is_pressed = 0;

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
			if (key == SDLK_w) w_is_pressed = 1;
			if (key == SDLK_SPACE) space_is_pressed = 1;
		}	break;

		case SDL_KEYUP:
		{
			SDL_Keycode key = event.key.keysym.sym;
			if (key == SDLK_a) a_is_down = 0;
			if (key == SDLK_d) d_is_down = 0;
			if (key == SDLK_w) w_is_down = 0;
			if (key == SDLK_s) s_is_down = 0;
		}	break;
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
	} else if (d_is_down) {
		player.vel.x = player_speed;
	} else {
		player.vel.x = 0;
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

	if(w_is_pressed && player.can_jump)
	{
		player.vel.y = player_jump_speed;
		player.can_jump = 0;
		player.on_ground = 0;
	}

	int hit_ground = 0;

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
			hit_ground = 1;
		}
	}
	if (iters == max_iters) printf("Failed to exhaust timestep.\n");

	float inv_dt = dt ? 1.0f / dt : 0;
	player.vel = vel * inv_dt;

	draw_capsule(player.capsule);

	if (player.on_ground) {
		player.vel.y = 0;
	}

	gl_line_color(gfx, 1.0f, 1.0f, 1.0f);
	draw_map(&map);

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

	//gl_vertex_data_t vd;
	//gl_make_vertex_data(&vd, 1024 * 1024, GL_TRIANGLES, sizeof(cute_font_vert_t), GL_DYNAMIC_DRAW);
	//gl_add_attribute(&vd, "in_pos", 2, CUTE_GL_FLOAT, CUTE_GL_OFFSET_OF(cute_font_vert_t, x));
	//gl_add_attribute(&vd, "in_uv", 2, CUTE_GL_FLOAT, CUTE_GL_OFFSET_OF(cute_font_vert_t, u));

	//gl_make_renderable(&font_renderable, &vd);
	//gl_load_shader(&font_shader, vs, ps);
	//gl_set_shader(&font_renderable, &font_shader);
	
	gl_ortho_2d((float)640 / 2.0f, (float)480 / 2.0f, 0, 0, projection);
	glViewport(0, 0, 640, 480);

	//gl_send_matrix(&font_shader, "u_mvp", projection);
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

	player.capsule.r = PLAYER_WIDTH;
	player.pos = v2(36.3215637f, 37.36820793f);

	while (application_running)
		main_loop();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

#include <glad/glad.c>

#define CUTE_C2_IMPLEMENTATION
#include <cute_c2.h>
