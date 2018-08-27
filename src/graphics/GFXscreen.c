#include "GFXscreen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define MATH_3D_IMPLEMENTATION
#include <math_3d/math_3d.h>


#define FNAME "GFXscreen.c"
#define INFO_LOG_SZ 2048


static void init_glfw(void);
static void create_window(GFXscreen gfxs);
static void init_glad(void);
static void framebuffer_resize_cback(GLFWwindow *win, int w, int h);

void create_program(GFXscreen gfxs, const char *vert_path,
	const char *frag_path);
static unsigned create_shader(const char *shader_path, GLenum shader_type);
static char* load_shader(const char *shader_path);

static void enable_pixel_coordinates(GFXscreen gfxs);

static void generate_vertices(GFXscreen gfxs, unsigned boarder_thickns);
static void generate_indices(GFXscreen gfxs);

static void create_vertex_array(GFXscreen gfxs);
static void create_array_buffer_pos(GFXscreen gfxs);
static void create_element_array_buffer(GFXscreen gfxs);

static struct Boarder* create_boarder(unsigned w, unsigned h, unsigned thickns,
	long color);
static void generate_boarder_vertices(struct Boarder *boarder, unsigned w,
	unsigned h, unsigned thickns);
static void generate_boarder_indices(struct Boarder *boarder);
static void generate_boarder_colors(struct Boarder *boarder, long color);
static void create_boarder_vertex_array(struct Boarder *boarder);
static void create_boarder_array_buffer_pos(struct Boarder *boarder);
static void create_boarder_element_array_buffer(struct Boarder *boarder);
static void create_boarder_array_buffer_col(struct Boarder *boarder);

static void generate_colors(GFXscreen gfxs, const unsigned char gfx[]);
static void create_array_buffer_col(GFXscreen gfxs);

static void destroy_boarder(struct Boarder *boarder);


static bool instance_exists = false;
static GFXscreen active_instance;


struct GFXscreen_t {
	unsigned w;
	unsigned h;
	char *title;
	GLFWwindow *win;
	bool window_close;
	unsigned program;
	unsigned gfx_w;
	unsigned gfx_h;
	size_t vertices_sz;
	float *vertices;
	float pixel_sz;
	size_t indices_sz;
	unsigned *indices;
	unsigned vertex_array;
	unsigned array_buffer_pos;
	unsigned element_array_buffer;
	Map keypad_keyboard_map;
	Map keypad_state_map;
	long color_on;
	long color_off;
	size_t colors_sz;
	float *colors;
	unsigned array_buffer_col;
	unsigned fps;
	double prev_frame;
	struct Boarder *boarder;
};

struct Boarder {
	unsigned width;
	size_t vertices_sz;
	float *vertices;
	size_t indices_sz;
	unsigned *indices;
	size_t colors_sz;
	float *colors;
	unsigned vertex_array;
	unsigned array_buffer_pos;
	unsigned element_array_buffer;
	unsigned array_buffer_col;
};


GFXscreen GFXscreen_create(unsigned w, unsigned h, const char *title,
	unsigned gfx_w, unsigned gfx_h, long color_on, long color_off,
	unsigned fps, unsigned boarder_thickns)
{
	if (instance_exists)
		exit_log(FNAME, 1,
			"Failed creating GFXscreen, only one instance permitted.");

	static bool graphics_modules_init;
	if (!graphics_modules_init)
		init_glfw();

	GFXscreen gfxs = (GFXscreen)malloc(sizeof(struct GFXscreen_t));
	if (!gfxs)
		exit_log(FNAME, 1,
			"Failed creating GFXscreen, memory allocation fail.");

	gfxs->w = w;
	gfxs->h = h;
	
	if (title) {
		size_t title_len = strlen(title);
		gfxs->title = (char*)malloc(sizeof(char) * (title_len + 1));
		if (!gfxs->title)
			exit_log(FNAME, 1,
				"Failed creating GFXsceen, memory allocation fail.");
		strncpy(gfxs->title, title, title_len);
		gfxs->title[title_len] = '\0';
	}
	else {
		gfxs->title = (char*)malloc(sizeof(char));
		if (!gfxs->title)
			exit_log(FNAME, 1,
				"Failed creating GFXsceen, memory allocation fail.");
		*(gfxs->title) = '\0';
	}

	create_window(gfxs);
	gfxs->window_close = false;

	if (!graphics_modules_init) {
		init_glad();
		graphics_modules_init = true;
	}

	glfwSetFramebufferSizeCallback(gfxs->win, framebuffer_resize_cback);

	create_program(gfxs, "../src/graphics/shader.vert",
    "../src/graphics/shader.frag");
	enable_pixel_coordinates(gfxs);

	gfxs->gfx_w = gfx_w;
	gfxs->gfx_h = gfx_h;
	// 4 vertices per pixel (tl, tr, bl, br), 2 coordinates per vertex (x, y)
	gfxs->vertices_sz = gfx_w * 4 * 2 * gfx_h;
	gfxs->vertices = (float*)malloc(sizeof(float) * gfxs->vertices_sz);
	if (!gfxs->vertices)
		exit_log(FNAME, 1,
			"Failed creating GFXscreen, memory allocation fail.");
	generate_vertices(gfxs, boarder_thickns);
	
	// 6 indices per pixel (3 for tr triangle and 3 for bl triangle)
	gfxs->indices_sz = gfx_w * gfx_h * 6;
	gfxs->indices = (unsigned*)malloc(sizeof(unsigned) * gfxs->indices_sz);
	generate_indices(gfxs);

	create_vertex_array(gfxs);
	gfxs->array_buffer_pos = 0;
	create_array_buffer_pos(gfxs);
	create_element_array_buffer(gfxs);

	gfxs->keypad_keyboard_map = map_create(0);
	gfxs->keypad_state_map = map_create(0);

	gfxs->color_on = color_on;
	gfxs->color_off = color_off;
	// 4 vertices per pixel (tl, tr, bl, br), 3 colors per vertex (r, g, b)
	gfxs->colors_sz = gfx_w * 4 * 3 * gfx_h;
	gfxs->colors = (float*)malloc(sizeof(float) * gfxs->colors_sz);
	if (!gfxs->colors)
		exit_log(FNAME, 1,
			"Failed creating GFXscreen, memory allocation fail.");
	gfxs->array_buffer_col = 0;

	gfxs->fps = fps;
	gfxs->prev_frame = 0.0;

	gfxs->boarder = create_boarder(gfx_w * gfxs->pixel_sz + boarder_thickns * 2,
		gfx_h * gfxs->pixel_sz + boarder_thickns * 2, boarder_thickns,
		color_on);

	instance_exists = true;
	active_instance = gfxs;
	return gfxs;
}

static void init_glfw(void)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

static void create_window(GFXscreen gfxs)
{
	gfxs->win = glfwCreateWindow(gfxs->w, gfxs->h, gfxs->title,
		NULL, NULL);
	if (!gfxs->win)
		exit_log(FNAME, 1, "Failed creating window, GLFW failed.");
    
    int primary_monitor_x;
    int primary_monitor_y;
    glfwGetMonitorPos(glfwGetPrimaryMonitor(), &primary_monitor_x,
        &primary_monitor_y);
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(
        gfxs->win,
        primary_monitor_x + (mode->width - gfxs->w) / 2,
		primary_monitor_y + (mode->height - gfxs->h) / 2
    );

	glfwMakeContextCurrent(gfxs->win);
    glfwSwapInterval(0);
}

static void init_glad(void)
{
	int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (status == 0)
		exit_log(FNAME, 1, "Failed initializing GLAD.");
}

static void framebuffer_resize_cback(GLFWwindow *win, int w, int h)
{
	glViewport(0, 0, w, h);
	active_instance->w = w;
	active_instance->h = h;
	enable_pixel_coordinates(active_instance);

	unsigned thickns = active_instance->boarder->width;

	generate_vertices(active_instance, thickns);
	create_array_buffer_pos(active_instance);

	generate_boarder_vertices(active_instance->boarder,
		active_instance->gfx_w * active_instance->pixel_sz + thickns * 2,
		active_instance->gfx_h * active_instance->pixel_sz + thickns * 2,
		thickns);
	create_boarder_array_buffer_pos(active_instance->boarder);
}

void create_program(GFXscreen gfxs, const char *vert_path, const char *frag_path)
{
	gfxs->program = glCreateProgram();
	unsigned vert_shader = create_shader(vert_path, GL_VERTEX_SHADER);
	unsigned frag_shader = create_shader(frag_path, GL_FRAGMENT_SHADER);
	glAttachShader(gfxs->program, vert_shader);
	glAttachShader(gfxs->program, frag_shader);
	glLinkProgram(gfxs->program);

	int status;
	glGetProgramiv(gfxs->program, GL_LINK_STATUS, &status);
	if (!status) {
		char info_log[INFO_LOG_SZ];
		glGetProgramInfoLog(gfxs->program, INFO_LOG_SZ, NULL, info_log);
		exit_log(FNAME, 2, "Failed linking program.", info_log);
	}

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);
	glUseProgram(gfxs->program);
}

static unsigned create_shader(const char *shader_path, GLenum shader_type)
{
	unsigned shader = glCreateShader(shader_type);
	char *shader_data = load_shader(shader_path);
	glShaderSource(shader, 1, (const char**)&shader_data, NULL);
	glCompileShader(shader);
	
	int status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char info_log[INFO_LOG_SZ];
		glGetShaderInfoLog(shader, INFO_LOG_SZ, NULL, info_log);
		if (shader_type == GL_VERTEX_SHADER)
			exit_log(FNAME, 2, "Failed compiling vertex shader.", info_log);
		else
			exit_log(FNAME, 2, "Failed compiling fragment shader.", info_log);
	}

	free(shader_data);
	return shader;
}

static char* load_shader(const char *shader_path)
{
	FILE *f = fopen(shader_path, "r");
	if (!f)
		exit_log(FNAME, 2, "Failed loading shader, bad path.", shader_path);

	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	rewind(f);

	char *data = (char*)malloc(sizeof(char) * len + 1);
	if (!data)
		exit_log(FNAME, 1, "Failed loading shader, memory allocation failed.");
	for (size_t i = 0; i < len - 1; ++i)
		data[i] = fgetc(f);
	data[len - 1] = '\0';

	fclose(f);
	return data;
}

static void enable_pixel_coordinates(GFXscreen gfxs)
{
	mat4_t ortho = m4_ortho(0, gfxs->w, gfxs->h, 0, 0, 1);
	glUniformMatrix4fv(glGetUniformLocation(gfxs->program, "ortho"), 1,
		GL_FALSE, &ortho.m00);
}

static void generate_vertices(GFXscreen gfxs, unsigned boarder_thickns)
{
	/* 
	 * determines the lesser of pixel height or pixel width, uses it as the
	 * unified size inorder to maximize window usage and keep pixels squared,
	 * includes padding required for border
	 */
	gfxs->pixel_sz =
		((float)gfxs->w - boarder_thickns * 2) / gfxs->gfx_w 
		<
		((float)gfxs->h - boarder_thickns * 2) / gfxs->gfx_h
			? ((float)gfxs->w - boarder_thickns * 2) / gfxs->gfx_w
			: ((float)gfxs->h - boarder_thickns * 2) / gfxs->gfx_h;
	float pixel_sz = gfxs->pixel_sz;

	size_t vertices_iter;
	for (size_t i = 0; i < gfxs->gfx_h; ++i) {
		for (size_t j = 0; j < gfxs->gfx_w; ++j) {
			vertices_iter = i * gfxs->gfx_w * 4 * 2 + j * 4 * 2;

			// top left vertex
			gfxs->vertices[vertices_iter++] =
				pixel_sz * j + boarder_thickns;            // x
			gfxs->vertices[vertices_iter++] =
				pixel_sz * i + boarder_thickns;            // y
			// top right vertex
			gfxs->vertices[vertices_iter++] =
				pixel_sz * j + pixel_sz + boarder_thickns; // x
			gfxs->vertices[vertices_iter++] =
				pixel_sz * i + boarder_thickns;			   // y
			// bottom left vertex
			gfxs->vertices[vertices_iter++] =
				pixel_sz * j + boarder_thickns;			   // x
			gfxs->vertices[vertices_iter++] =
				pixel_sz * i + pixel_sz + boarder_thickns; // y
			// bottom right vertex
			gfxs->vertices[vertices_iter++] =
				pixel_sz * j + pixel_sz + boarder_thickns; // x
			gfxs->vertices[vertices_iter  ] =
				pixel_sz * i + pixel_sz + boarder_thickns; // y
		}
	}
}

static void generate_indices(GFXscreen gfxs)
{
	size_t indices_iter;
	for (size_t i = 0; i < gfxs->gfx_h; ++i) {
		for (size_t j = 0; j < gfxs->gfx_w; ++j) {
			indices_iter = i * gfxs->gfx_w * 6 + j * 6;

			// top right triangle
			gfxs->indices[indices_iter++] = i * gfxs->gfx_w * 4 + j * 4;
			gfxs->indices[indices_iter++] = i * gfxs->gfx_w * 4 + j * 4 + 1;
			gfxs->indices[indices_iter++] = i * gfxs->gfx_w * 4 + j * 4 + 3;
			// bottom left triangle
			gfxs->indices[indices_iter++] = i * gfxs->gfx_w * 4 + j * 4;
			gfxs->indices[indices_iter++] = i * gfxs->gfx_w * 4 + j * 4 + 2;
			gfxs->indices[indices_iter  ] = i * gfxs->gfx_w * 4 + j * 4 + 3;
		}
	}
}

static void create_vertex_array(GFXscreen gfxs)
{
	glGenVertexArrays(1, &gfxs->vertex_array);
	glBindVertexArray(gfxs->vertex_array);
}

static void create_array_buffer_pos(GFXscreen gfxs)
{
	if(!gfxs->array_buffer_pos)
		glGenBuffers(1, &gfxs->array_buffer_pos);

	glBindBuffer(GL_ARRAY_BUFFER, gfxs->array_buffer_pos);
	glBufferData(GL_ARRAY_BUFFER, gfxs->vertices_sz * sizeof(float),
		gfxs->vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
}

static void create_element_array_buffer(GFXscreen gfxs)
{
	glGenBuffers(1, &gfxs->element_array_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gfxs->element_array_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, gfxs->indices_sz * sizeof(unsigned),
		gfxs->indices, GL_STATIC_DRAW);
}

static struct Boarder* create_boarder(unsigned w, unsigned h, unsigned thickns,
	long color)
{
	struct Boarder *boarder = (struct Boarder*)malloc(sizeof(struct Boarder));
	if (!boarder)
		exit_log(FNAME, 1, "Failed creating Boarder, memory allocation fail.");

	boarder->width = thickns;

	// 4 vertices per edge (tl, tr, bl, br), 2 coordinates per vertex (x, y)
	boarder->vertices_sz = 4 * 4 * 2;
	boarder->vertices = (float*)malloc(sizeof(float) * boarder->vertices_sz);
	if (!boarder->vertices)
		exit_log(FNAME, 1, "Failed creating Boarder, memory allocation fail.");
	generate_boarder_vertices(boarder, w, h, thickns);

	// 6 indices per edge (3 for tr triangle and 3 for bl triangle)
	boarder->indices_sz = 4 * 6;
	boarder->indices = 
        (unsigned*)malloc(sizeof(unsigned) * boarder->indices_sz);
	if (!boarder->indices)
		exit_log(FNAME, 1, "Failed creating Boarder, memory allocation fail.");
	generate_boarder_indices(boarder);

	// 4 vertices per edge (tl, tr, bl, br), 3 colors per vertex (r, g, b)
	boarder->colors_sz = 4 * 4 * 3;
	boarder->colors = (float*)malloc(sizeof(float) * boarder->colors_sz);
	if (!boarder->colors)
		exit_log(FNAME, 1, "Failed creating Boarder, memory allocation fail.");
	generate_boarder_colors(boarder, color);

	create_boarder_vertex_array(boarder);
	boarder->array_buffer_pos = 0;
	create_boarder_array_buffer_pos(boarder);
	create_boarder_element_array_buffer(boarder);
	create_boarder_array_buffer_col(boarder);

	return boarder;
}

static void generate_boarder_vertices(struct Boarder *boarder, unsigned w,
	unsigned h, unsigned thickns)
{
	float *iter = boarder->vertices;

	// top boarder
	*iter++ = 0.0f;	       *iter++ = 0.0f;        // TL ver
	*iter++ = w;	       *iter++ = 0.0f;        // TR ver
	*iter++ = 0.0f;        *iter++ = thickns;     // BL ver
	*iter++ = w;           *iter++ = thickns;     // BR ver
	// right boarder
	*iter++ = w - thickns; *iter++ = 0.0f;        // TL ver
	*iter++ = w;           *iter++ = 0.0f;        // TR ver
	*iter++ = w - thickns; *iter++ = h;           // BL ver
	*iter++ = w;           *iter++ = h;           // BR ver
	// bottom boarder
	*iter++ = 0.0f;	       *iter++ = h - thickns; // TL ver
	*iter++ = w;	       *iter++ = h - thickns; // TR ver
	*iter++ = 0.0f;        *iter++ = h;           // BL ver
	*iter++ = w;           *iter++ = h;           // BR ver
	// left boarder
	*iter++ = 0.0f;        *iter++ = 0.0f;        // TL ver
	*iter++ = thickns;     *iter++ = 0.0f;        // TR ver
	*iter++ = 0.0f;        *iter++ = h;           // BL ver
	*iter++ = thickns;     *iter++ = h;           // BR ver
}

static void generate_boarder_indices(struct Boarder *boarder)
{
	for (size_t i = 0; i < boarder->indices_sz;) {
		// top right triangle
		boarder->indices[i++] = i / 6 * 4;
		boarder->indices[i++] = i / 6 * 4 + 1;
		boarder->indices[i++] = i / 6 * 4 + 3;
		// bottom left triangle
		boarder->indices[i++] = i / 6 * 4;
		boarder->indices[i++] = i / 6 * 4 + 2;
		boarder->indices[i++] = i / 6 * 4 + 3;
	}
}

static void generate_boarder_colors(struct Boarder *boarder, long color)
{
	for (size_t i = 0; i < boarder->colors_sz;) {
		boarder->colors[i++] = (color & 0xFF0000) >> 16; // r
		boarder->colors[i++] = (color & 0x00FF00) >> 8;  // g
		boarder->colors[i++] = color & 0x0000FF;         // b
	}
}

static void create_boarder_vertex_array(struct Boarder *boarder)
{
	glGenVertexArrays(1, &boarder->vertex_array);
	glBindVertexArray(boarder->vertex_array);
}

static void create_boarder_array_buffer_pos(struct Boarder *boarder)
{
	if(!boarder->array_buffer_pos)
		glGenBuffers(1, &boarder->array_buffer_pos);

	glBindBuffer(GL_ARRAY_BUFFER, boarder->array_buffer_pos);
	glBufferData(GL_ARRAY_BUFFER, boarder->vertices_sz * sizeof(float),
		boarder->vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
}

static void create_boarder_element_array_buffer(struct Boarder *boarder)
{
	glGenBuffers(1, &boarder->element_array_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boarder->element_array_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		boarder->indices_sz * sizeof(unsigned), boarder->indices,
		GL_STATIC_DRAW);
}

static void create_boarder_array_buffer_col(struct Boarder *boarder)
{
	glGenBuffers(1, &boarder->array_buffer_col);
	glBindBuffer(GL_ARRAY_BUFFER, boarder->array_buffer_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * boarder->colors_sz,
		boarder->colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
}

bool GFXscreen_window_close(GFXscreen gfxs)
{
	return gfxs->window_close;
}

void GFXscreen_map_keypad_keyboard(GFXscreen gfxs, unsigned keypad,
	char keyboard)
{
	/*
	 * only 'Printable keys' are supported, GLFW documentation on GLFW_KEY_X
	 * https://github.com/glfw/glfw/blob/master/include/GLFW/glfw3.h
	 */
	if (!(
		keyboard == 32
		||
		keyboard == 39
		||
		keyboard >= 44 && keyboard <= 57
		||
		keyboard == 59
		||
		keyboard == 61
		||
		keyboard >= 65 && keyboard <= 93
		||
		keyboard == 96
	)) {
		char keyboard_str[32];
		sprintf(keyboard_str, "\tkey: %c", keyboard);
		exit_log(FNAME, 1, "Failed mapping keypad button, unsupported key.",
			keyboard_str);
	}
	
	map_add(gfxs->keypad_keyboard_map, keypad, keyboard);
	map_add(gfxs->keypad_state_map, keypad, 0);
}

void GFXscreen_process_input(GFXscreen gfxs)
{
	glfwPollEvents();

	if (glfwWindowShouldClose(gfxs->win))
		gfxs->window_close = true;

	if (glfwGetKey(gfxs->win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(gfxs->win, 1);

	const int *keys = map_get_keys(gfxs->keypad_keyboard_map);
	for (size_t i = 0; i < map_get_size(gfxs->keypad_keyboard_map); ++i)
		if (
			glfwGetKey(gfxs->win, map_get(gfxs->keypad_keyboard_map, keys[i]))
			==
			GLFW_PRESS
		)
			map_set(gfxs->keypad_state_map, keys[i], 1);
		else
			map_set(gfxs->keypad_state_map, keys[i], 0);
}

const Map GFXscreen_get_keypad_state_map(GFXscreen gfxs)
{
	return gfxs->keypad_state_map;
}

void GFXscreen_draw_frame(GFXscreen gfxs, const unsigned char gfx[])
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(gfxs->vertex_array);
	generate_colors(gfxs, gfx);
	create_array_buffer_col(gfxs);
	glDrawElements(GL_TRIANGLES, gfxs->indices_sz, GL_UNSIGNED_INT, NULL);

	glBindVertexArray(gfxs->boarder->vertex_array);
	glDrawElements(GL_TRIANGLES, gfxs->boarder->indices_sz, GL_UNSIGNED_INT,
		NULL);

	glfwSwapBuffers(gfxs->win);

	// synchronize the frame rate
    double frame_duration = 1000.0 / gfxs->fps / 1000.0;
	while (glfwGetTime() - gfxs->prev_frame < frame_duration)
        ;
	gfxs->prev_frame = glfwGetTime();
}

static void generate_colors(GFXscreen gfxs, const unsigned char gfx[])
{
	long color;
	size_t colors_iter;
	for (size_t i = 0; i < gfxs->gfx_h; ++i) {
		for (size_t j = 0; j < gfxs->gfx_w; ++j) {
			color = gfx[i * gfxs->gfx_w + j] ? gfxs->color_on : gfxs->color_off;
			colors_iter = i * gfxs->gfx_w * 4 * 3 + j * 4 * 3;

			// top left vertex
			gfxs->colors[colors_iter++] = ((color & 0xFF0000) >> 16) / 255.0;//r
			gfxs->colors[colors_iter++] = ((color & 0x00FF00) >> 8) / 255.0; //g
			gfxs->colors[colors_iter++] = (color & 0x0000FF) / 255.0f;		 //b
			// top right vertex
			gfxs->colors[colors_iter++] = ((color & 0xFF0000) >> 16) / 255.0;//r
			gfxs->colors[colors_iter++] = ((color & 0x00FF00) >> 8) / 255.0; //g
			gfxs->colors[colors_iter++] = (color & 0x0000FF) / 255.0;		 //b
			// bottom left vertex
			gfxs->colors[colors_iter++] = ((color & 0xFF0000) >> 16) / 255.0;//r
			gfxs->colors[colors_iter++] = ((color & 0x00FF00) >> 8) / 255.0; //g
			gfxs->colors[colors_iter++] = (color & 0x0000FF) / 255.0;		 //b
			// bottom right vertex
			gfxs->colors[colors_iter++] = ((color & 0xFF0000) >> 16) / 255.0;//r
			gfxs->colors[colors_iter++] = ((color & 0x00FF00) >> 8) / 255.0; //g
			gfxs->colors[colors_iter++] = (color & 0x0000FF) / 255.0;		 //b
		}
	}
}

static void create_array_buffer_col(GFXscreen gfxs)
{
	if(!gfxs->array_buffer_col)
		glGenBuffers(1, &gfxs->array_buffer_col);

	glBindBuffer(GL_ARRAY_BUFFER, gfxs->array_buffer_col);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * gfxs->colors_sz, gfxs->colors,
		GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
}

void GFXscreen_destroy(GFXscreen gfxs)
{
	destroy_boarder(gfxs->boarder);
	glDeleteBuffers(1, &gfxs->array_buffer_col);
	free(gfxs->colors);
	map_destroy(gfxs->keypad_state_map);
	map_destroy(gfxs->keypad_keyboard_map);
	glDeleteBuffers(1, &gfxs->element_array_buffer);
	glDeleteBuffers(1, &gfxs->array_buffer_pos);
	glDeleteVertexArrays(1, &gfxs->vertex_array);
	free(gfxs->indices);
	free(gfxs->vertices);
	glDeleteProgram(gfxs->program);
	glfwDestroyWindow(gfxs->win);
	glfwTerminate();
	free(gfxs);
	instance_exists = false;
	active_instance = NULL;
}

static void destroy_boarder(struct Boarder *boarder)
{
	glDeleteBuffers(1, &boarder->array_buffer_col);
	glDeleteBuffers(1, &boarder->element_array_buffer);
	glDeleteBuffers(1, &boarder->array_buffer_pos);
	glDeleteVertexArrays(1, &boarder->vertex_array);
	free(boarder->colors);
	free(boarder->indices);
	free(boarder->vertices);
	free(boarder);
}
