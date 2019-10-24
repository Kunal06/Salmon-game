// Header
#include "salmon.hpp"

// internal
#include "turtle.hpp"
#include "fish.hpp"
#include "shark.hpp"

// stlib
#include <vector>
#include <string>
#include <algorithm>

bool is_up = false;
bool is_down = false;
bool is_left = false;
bool is_right = false;
vec2 translation_vec;

bool Salmon::init()
{
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	// Reads the salmon mesh from a file, which contains a list of vertices and indices
	FILE *mesh_file = fopen(mesh_path("salmon.mesh"), "r");
	if (mesh_file == nullptr)
		return false;

	// Reading vertices and colors
	size_t num_vertices;
	fscanf(mesh_file, "%zu\n", &num_vertices);
	for (size_t i = 0; i < num_vertices; ++i)
	{
		float x, y, z;
		float _u[3]; // unused
		int r, g, b;
		fscanf(mesh_file, "%f %f %f %f %f %f %d %d %d\n", &x, &y, &z, _u, _u + 1, _u + 2, &r, &g, &b);
		Vertex vertex;
		vertex.position = {x, y, -z};
		vertex.color = {(float)r / 255, (float)g / 255, (float)b / 255};
		vertices.push_back(vertex);
	}

	// Reading associated indices
	size_t num_indices;
	fscanf(mesh_file, "%zu\n", &num_indices);
	for (size_t i = 0; i < num_indices; ++i)
	{
		int idx[3];
		fscanf(mesh_file, "%d %d %d\n", idx, idx + 1, idx + 2);
		indices.push_back((uint16_t)idx[0]);
		indices.push_back((uint16_t)idx[1]);
		indices.push_back((uint16_t)idx[2]);
	}

	// Done reading
	fclose(mesh_file);

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("salmon.vs.glsl"), shader_path("salmon.fs.glsl")))
		return false;

	// Setting initial values
	motion.position = {50.f, 100.f};
	motion.radians = 0.f;
	motion.speed = 200.f;

	physics.scale = {-35.f, 35.f};

	m_is_alive = true;
	m_light_up_countdown_ms = -1.f;

	return true;
}

// Releases all graphics resources
void Salmon::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

// Called on each frame by World::update()
void Salmon::update(float ms)
{
	const float SALMON_SPEED = 200.f;
	float step = SALMON_SPEED * (ms / 1000);
	vec2 up_vec = {0.f, -10.f};
	vec2 down_vec = {0.f, 10.f};
	vec2 left_vec = {-10.f, 0.f};
	vec2 right_vec = {10.f, 0.f};
	float off = 0.05;
	if (m_is_alive)
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// UPDATE SALMON POSITION HERE BASED ON KEY PRESSED (World::on_key())
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if (is_up)
		{
			move(up_vec);
		}
		if (is_down)
		{
			move(down_vec);
		}
		if (is_left)
		{
			rotate(off);
		}
		if (is_right)
		{
			rotate(-off);
		}
	}
	else
	{
		// If dead we make it face upwards and sink deep down
		set_rotation(3.1415f);
		move({0.f, step});
	}

	if (m_light_up_countdown_ms > 0.f)
		m_light_up_countdown_ms -= ms;
}

void Salmon::draw(const mat3 &projection)
{
	transform.begin();

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// SALMON TRANSFORMATION CODE HERE

	// see Transformations and Rendering in the specification pdf
	// the following functions are available:
	// translate()
	// rotate()
	// scale()

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// REMOVE THE FOLLOWING LINES BEFORE ADDING ANY TRANSFORMATION CODE
	// transform.translate({ 100.0f, 100.0f });
	// transform.scale(physics.scale);
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	transform.translate({motion.position.x, motion.position.y});
	transform.rotate(motion.radians);
	transform.scale(physics.scale);
	transform.end();

	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	// Getting uniform locations
	GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
	GLint color_uloc = glGetUniformLocation(effect.program, "fcolor");
	GLint projection_uloc = glGetUniformLocation(effect.program, "projection");
	GLint light_up_uloc = glGetUniformLocation(effect.program, "light_up");

	// Setting vertices and indices
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
	GLint in_color_loc = glGetAttribLocation(effect.program, "in_color");
	glEnableVertexAttribArray(in_position_loc);
	glEnableVertexAttribArray(in_color_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
	glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)sizeof(vec3));

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float *)&transform.out);

	// !!! Salmon Color
	float color[] = {1.f, 1.f, 1.f};
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float *)&projection);

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HERE TO SET THE CORRECTLY LIGHT UP THE SALMON IF HE HAS EATEN RECENTLY
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	int light_up = (m_light_up_countdown_ms > 0.f) ? 1 : 0;
	glUniform1iv(light_up_uloc, 1, &light_up);

	float red_color[] = {255.f, 0.f, 0.f};

	// When the salmon gets hit by a turtle or a shark salmon turns red
	if (!is_alive())
	{
		glUniform3fv(color_uloc, 1, red_color);
	}
	// Get number of infices from buffer,
	// we know our vbo contains both colour and position information, so...
	GLint size = 0;
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	GLsizei num_indices = size / sizeof(uint16_t);

	// Drawing!
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
}

// Simple bounding box collision check
bool Salmon::collides_with(const Turtle &turtle)
{
	float dx = motion.position.x - turtle.get_position().x;
	float dy = motion.position.y - turtle.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(turtle.get_bounding_box().x, turtle.get_bounding_box().y);
	float my_r = std::max(physics.scale.x, physics.scale.y);
	float r = std::max(other_r, my_r);
	r *= 0.6f;
	if (d_sq < r * r)
		return true;
	return false;
}

bool Salmon::collides_with(const Fish &fish)
{
	float dx = motion.position.x - fish.get_position().x;
	float dy = motion.position.y - fish.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(fish.get_bounding_box().x, fish.get_bounding_box().y);
	float my_r = std::max(physics.scale.x, physics.scale.y);
	float r = std::max(other_r, my_r);
	r *= 0.6f;
	if (d_sq < r * r)
		return true;
	return false;
}
// Shark Added
bool Salmon::collides_with(const Shark &shark)
{
	float dx = motion.position.x - shark.get_position().x;
	float dy = motion.position.y - shark.get_position().y;
	float d_sq = dx * dx + dy * dy;
	float other_r = std::max(shark.get_bounding_box().x, shark.get_bounding_box().y);
	float my_r = std::max(physics.scale.x, physics.scale.y);
	float r = std::max(other_r, my_r);
	r *= 0.4f;
	if (d_sq < r * r)
		return true;
	return false;
}
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// HANDLE SALMON - WALL COLLISIONS HERE
// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
// You will want to write new functions from scratch for checking/handling
// salmon - wall collisions.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

vec2 Salmon::get_position() const
{
	return motion.position;
}

void Salmon::move(vec2 off)
{
	motion.position.x += off.x;
	motion.position.y += off.y;
	translation_vec = {motion.position.x, motion.position.y};
}

void Salmon::set_rotation(float radians)
{
	motion.radians = radians;
}

void Salmon::rotate(float off)
{
	// move in direction
	// float rotate = (GLfloat)atan2(motion.position.x += off.x, motion.position.y += off.y);
	// float cs, sn, theta;
	// theta = 360 * 4.0 * atan(1.0) / 180.0;
	// cs = cos(theta);
	// sn = sin(theta);
	// vec2 off = {4.f, 4.f};
	// float rotate = (GLfloat)atan2(motion.position.x * cs - motion.position.y * sn, motion.position.x * sn - motion.position.y * cs);
	// fprintf(stdout, "ROTATE by %f\n", rotate);
	motion.radians += off; //ROTATE TO MOUSE
}

bool Salmon::is_alive() const
{
	return m_is_alive;
}

// Set salmon movement flag (used to keep track of the action and release(f) of the user keys)
void Salmon::set_movement(const std::string &flag)
{
	if (flag == "up")
	{
		is_up = true;
	}

	if (flag == "upf")
	{
		is_up = false;
	}

	if (flag == "down")
	{
		fprintf(stdout, "set_movement:::keypress down\n");
		is_down = true;
	}
	if (flag == "downf")
	{
		is_down = false;
	}

	if (flag == "left")
	{
		is_left = true;
	}
	if (flag == "leftf")
	{
		is_left = false;
	}

	if (flag == "right")
	{
		is_right = true;
	}

	if (flag == "rightf")
	{
		is_right = false;
	}
}

// Get salmon movement flag (used to keep track of the movement)
bool Salmon::get_movement(const std::string &flag)
{
	if (flag == "up")
	{
		return is_up;
	}
	else if (flag == "down")
	{
		return is_down;
	}
	else if (flag == "left")
	{
		return is_left;
	}
	else if (flag == "right")
	{
		return is_right;
	}
	// check
	return is_up;
}

// Called when the salmon collides with a turtle or shark
void Salmon::kill()
{
	m_is_alive = false;
}

// Called when the salmon collides with a fish
void Salmon::light_up()
{
	m_light_up_countdown_ms = 1500.f;
}
