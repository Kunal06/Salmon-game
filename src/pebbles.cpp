// Header
#include "pebbles.hpp"
#include "turtle.hpp"
#include "fish.hpp"

#include <cmath>
#include <iostream>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// DON'T WORRY ABOUT THIS CLASS UNTIL ASSIGNMENT 3
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

static const int MAX_PEBBLES = 25;
static const int MAX_COOLDOWN = 20;
int COOLDOWN = 0;
constexpr int NUM_SEGMENTS = 12;

bool Pebbles::init()
{
	std::vector<GLfloat> screen_vertex_buffer_data;
	constexpr float z = -0.1;

	for (int i = 0; i < NUM_SEGMENTS; i++)
	{
		screen_vertex_buffer_data.push_back(std::cos(M_PI * 2.0 * float(i) / (float)NUM_SEGMENTS));
		screen_vertex_buffer_data.push_back(std::sin(M_PI * 2.0 * float(i) / (float)NUM_SEGMENTS));
		screen_vertex_buffer_data.push_back(z);

		screen_vertex_buffer_data.push_back(std::cos(M_PI * 2.0 * float(i + 1) / (float)NUM_SEGMENTS));
		screen_vertex_buffer_data.push_back(std::sin(M_PI * 2.0 * float(i + 1) / (float)NUM_SEGMENTS));
		screen_vertex_buffer_data.push_back(z);

		screen_vertex_buffer_data.push_back(0);
		screen_vertex_buffer_data.push_back(0);
		screen_vertex_buffer_data.push_back(z);
	}

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, screen_vertex_buffer_data.size() * sizeof(GLfloat), screen_vertex_buffer_data.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &m_instance_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);

	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("pebble.vs.glsl"), shader_path("pebble.fs.glsl")))
		return false;
	motion.speed = 100.f;
	return true;
}

// Releases all graphics resources
void Pebbles::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &m_instance_vbo);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);

	m_pebbles.clear();
}

void Pebbles::update(float ms)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PEBBLE UPDATES HERE
	// You will need to handle both the motion of pebbles
	// and the removal of dead pebbles.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	for (auto &pebble : m_pebbles)
	{
		// fprintf(stderr, "\nPebble - update - %f \n ", pebble.angle);
		// Add Gravity
		vec2 a = {0.f, 5.81f};
		float dt = (ms) / 1000;

		// HORIZONTAL
		// v = u + at, a=0
		// pebble.velocity.x = pebble.velocity.x + acceleration.x * dt;
		// // delta x = v*t
		// float step_x = pebble.velocity.x * dt;
		// pebble.position.x += step_x * cos(pebble.angle) ;

		// HORIZONTAL
		float step_x = pebble.velocity.x * dt;
		pebble.velocity.x -= step_x;
		pebble.position.x += step_x;

		// VERTICAL
		float delta_y = 800 - pebble.position.y ;
		float a_y = a.y;
		float Vy_i = pebble.velocity.y;
			// y = ut + 1/2 a t^2 , t= dt
		float step_y = Vy_i *dt + 1/2 * a.y * pow(dt,2);
		pebble.velocity.y += step_y;
		pebble.position.y += step_y;







		// new velocity in direction y , v = u + at
		// int initial_velocity_y = pebble.velocity.y;
		// pebble.velocity.y += acceleration.y * dt;
		// // delta y = (v+u)/2 *t
		// float step_y =  pebble.velocity.y * dt;
		// fprintf(stderr, "\nPebble - update - %f \n ", pebble.position.y);
		// pebble.position.y += step_y * sin(0);
	}
}

void Pebbles::spawn_pebble(vec2 position, float angle)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PEBBLE SPAWNING HERE
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (COOLDOWN == 0)
	{
		float PI = 3.14159;
		float velocity = (float(rand()) / (float(RAND_MAX) / (800.f - 500.f))) + 100.f;
		// fprintf(stderr, "\nPebble - update - %f \n ", angle);

		Pebble peb;
		peb.position.x = position.x + 55 * cos(angle);
		peb.position.y = position.y + 55 * sin(angle);
		angle = fmod (angle,PI/2);
		float MAX = angle + PI / 2;
		float MIN = angle - PI / 2;
		float direction = (float(rand()) / (float(RAND_MAX) / (MAX - MIN))) + MIN;
		peb.radius = 10;
		peb.velocity.x = 500.f;
		peb.velocity.y = 200.f;
		peb.angle = 0;
		m_pebbles.emplace_back(peb);
		COOLDOWN = 4;
	}
	else
	{
		COOLDOWN--;
	}
	// fprintf(stderr, "pebble spawned pebblie file \n");
}

void Pebbles::collides_with()
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PEBBLE COLLISIONS HERE
	// You will need to write additional functions from scratch.
	// Make sure to handle both collisions between pebbles
	// and collisions between pebbles and salmon/fish/turtles.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	int i = 0;

	for (auto &pebble : m_pebbles)
	{
		int j = 0;
		for (auto &pebble1 : m_pebbles)
		{
			if (i != j)
			{
				float dx = pebble.position.x - pebble1.position.x;
				float dy = pebble.position.y - pebble1.position.y;
				float d_sq = dx * dx + dy * dy;
				float other_r = pebble1.radius;
				float my_r = pebble.radius;
				float r = std::max(other_r, my_r);
				r *= 0.6f;
				if (d_sq < r * r)
				{
					pebble.angle = 3.14 - pebble.angle;
					//fprintf(stderr, "\nPebble - Collides - %f \n ", pebble.angle);
				}
			}
		}
	}
}

void Pebbles::collides_with(const Turtle &turtle)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PEBBLE COLLISIONS HERE
	// You will need to write additional functions from scratch.
	// Make sure to handle both collisions between pebbles
	// and collisions between pebbles and salmon/fish/turtles.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	for (auto &pebble : m_pebbles)
	{
		float dx = pebble.position.x - turtle.get_position().x;
		float dy = pebble.position.y - turtle.get_position().y;
		float d_sq = dx * dx + dy * dy;
		float other_r = std::max(turtle.get_bounding_box().x, turtle.get_bounding_box().y);
		float my_r = pebble.radius;
		float r = std::max(other_r, my_r);
		r *= 0.6f;
		if (d_sq < r * r)
		{
			pebble.angle = 3.14 - pebble.angle;
			fprintf(stderr, "\nPebble - Collides - %f \n ", pebble.angle);
		}
	}
}

// Draw pebbles using instancing
void Pebbles::draw(const mat3 &projection)
{
	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	// Getting uniform locations
	GLint projection_uloc = glGetUniformLocation(effect.program, "projection");
	GLint color_uloc = glGetUniformLocation(effect.program, "color");

	// Pebble color
	float color[] = {0.4f, 0.4f, 0.4f};
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float *)&projection);

	// Draw the screen texture on the geometry
	// Setting vertices
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	// Mesh vertex positions
	// Bind to attribute 0 (in_position) as in the vertex shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glVertexAttribDivisor(0, 0);

	// Load up pebbles into buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_pebbles.size() * sizeof(Pebble), m_pebbles.data(), GL_DYNAMIC_DRAW);

	// Pebble translations
	// Bind to attribute 1 (in_translate) as in vertex shader
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Pebble), (GLvoid *)offsetof(Pebble, position));
	glVertexAttribDivisor(1, 1);

	// Pebble radii
	// Bind to attribute 2 (in_scale) as in vertex shader
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Pebble), (GLvoid *)offsetof(Pebble, radius));
	glVertexAttribDivisor(2, 1);

	// Draw using instancing
	// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDrawArraysInstanced.xhtml
	glDrawArraysInstanced(GL_TRIANGLES, 0, NUM_SEGMENTS * 3, m_pebbles.size());

	// Reset divisor
	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(2, 0);
}