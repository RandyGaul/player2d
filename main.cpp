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

#define SERIALIZE_NO_UNIT_TESTS
#define CUTE_SERIALIZE_IMPLEMENTATION
#include <cute_serialize.h>

#include <cute_coroutine.h>
#include <cute_math2d.h>

#include <player2d.h>
#include <iostream>

int application_running = 1;
SDL_Window* window;
gl_context_t* gfx;
//gl_shader_t font_shader;
//gl_renderable_t font_renderable;
float projection[16];

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

	return (float)((double)(now - prev) * inv_freq);
}

void swap_buffers()
{
	SDL_GL_SwapWindow(window);
}

void main_loop()
{
	float dt = calc_dt();

	glClear(GL_COLOR_BUFFER_BIT);

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			application_running = 0;
			break;
		}
	}

	gl_line(gfx, 0, 0, 0, 100, 100, 0);

	gl_flush(gfx, swap_buffers, 0, 640, 480);
}

void sdl_setup()
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("player2d character controller demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

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
	std::cout << "12" << std::endl;

	int clear_bits = GL_COLOR_BUFFER_BIT;
	int settings_bits = 0;
	gfx = gl_make_ctx(32, clear_bits, settings_bits);

	std::cout << "13" << std::endl;

#define SHADER_STR(x) "#version 300 es\n" #x

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

	while (application_running)
		main_loop();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

#include <glad/glad.c>
