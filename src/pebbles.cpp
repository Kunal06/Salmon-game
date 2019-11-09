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
constexpr int NUM_SEGMENTS = 12;
float PI = 3.14159;

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

	collides_with(ms / 1000);

	for (auto &pebble : m_pebbles)
	{
		//fprintf(stderr, "\nPebble - update - %f \n ", pebble.velocity.x);
		//acceleration
		vec2 a = {0, 9.81};
		//collision test
		//vec2 a = {0, 0};

		float dt = ms / 1000;
		// v = u + at
		// horizontal velocity
		// fprintf(stderr, "\n after Pebble - Collides - %f - with - %f \n ", pebble.velocity.x, pebble.velocity.y);
		pebble.velocity.x = (pebble.velocity.x + a.x);
		//vertical velocity
		pebble.velocity.y = pebble.velocity.y + a.y;
		float step_x = 1.0 * pebble.velocity.x * dt;
		float step_y = 1.0 * pebble.velocity.y * dt;
		pebble.position.x += step_x;
		pebble.position.y += step_y;

		// REFLECT
	}

	// Removing out off screen pebbles
	auto pebble_it = m_pebbles.begin();
	while (pebble_it != m_pebbles.end())
	{
		float w = pebble_it->position.x / 2;
		if (pebble_it->position.x + w < 0.f)
		{
			pebble_it = m_pebbles.erase(pebble_it);
			continue;
		}

		++pebble_it;
	}
}

void Pebbles::spawn_pebble(vec2 position, float angle)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PEBBLE SPAWNING HERE
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//Randomize variables
	float velocity_x = (float(rand()) / (float(RAND_MAX) / (800.f - 500.f))) + 500.f;
	float velocity_y = (float(rand()) / (float(RAND_MAX) / (800.f - 500.f))) + 500.f;
	float MAX = angle + PI / 4;
	float MIN = angle - PI / 4;
	angle = (float(rand()) / (float(RAND_MAX) / (MAX - MIN))) + MIN;
	//fprintf(stderr, "\nPebble - update - %f \n ", angle);
	//Randomize variables -----

	Pebble peb;
	peb.angle = angle;
	peb.position.x = position.x + 55 * cos(angle);
	peb.position.y = position.y + 55 * sin(angle);
	peb.radius = (float(rand()) / (float(RAND_MAX) / (20 - 10))) + 10;
	peb.velocity.x = velocity_x * cos(peb.angle);
	peb.velocity.y = velocity_y * sin(peb.angle);
	peb.mass = 1.f;
	m_pebbles.emplace_back(peb);

	// Collision testing
	// Pebble peb;
	// peb.position.x = position.x;
	// peb.position.y = position.y;
	// peb.radius = 10;
	// peb.velocity.x = 500.f * cos(angle);
	// peb.velocity.y = 0 * sin(angle);
	// peb.mass = 1.f;
	// m_pebbles.emplace_back(peb);

	// Pebble peb1;
	// peb1.position.x = 1200 - position.x;
	// peb1.position.y = position.y;
	// peb1.radius = 10;
	// peb1.velocity.x = -400.f * cos(angle);
	// peb1.velocity.y = 0 * sin(angle);
	// peb1.mass = 1.f;
	// m_pebbles.emplace_back(peb1);

	//fprintf(stderr, "pebble spawned pebblie file \n");
}

void Pebbles::collides_with(float ms)
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
				vec2 p_v = pebble.velocity;
				vec2 p_pos = pebble.position;
				vec2 p1_v = pebble1.velocity;
				vec2 p1_pos = pebble1.position;
				//  figure out the distance between the two circles' centers. Pythagoras
				float a = p_pos.x - p1_pos.x;
				float b = p_pos.y - p1_pos.y;
				float c = sqrt(a * a + b * b);
				if (c < (pebble.radius + pebble1.radius))
				{
					//accurate collision points
					float collisionPointX =
						((p_pos.x * pebble1.radius) + (p1_pos.x * pebble.radius)) / (pebble.radius + pebble1.radius);
					float collisionPointY =
						((p_pos.y * pebble1.radius) + (p1_pos.y * pebble.radius)) / (pebble.radius + pebble1.radius);
					float pebble_velocity_x = (pebble.velocity.x * (pebble.mass - pebble1.mass) + (2 * pebble1.mass * pebble1.velocity.x)) / (pebble.mass + pebble1.mass);
					float pebble_velocity_y = (pebble.velocity.y * (pebble.mass - pebble1.mass) + (2 * pebble1.mass * pebble1.velocity.y)) / (pebble.mass + pebble1.mass);
					float pebble1_velocity_x = (pebble1.velocity.x * (pebble1.mass - pebble.mass) + (2 * pebble.mass * pebble.velocity.x)) / (pebble.mass + pebble1.mass);
					float pebble1_velocity_y = (pebble1.velocity.y * (pebble1.mass - pebble.mass) + (2 * pebble.mass * pebble.velocity.y)) / (pebble.mass + pebble1.mass);
					//fprintf(stderr, "\nAfter Pebble - Collides - %f - with - %f \n ", pebble_velocity_x, pebble1_velocity_x);
					float bounce = 1.0;
					pebble.velocity.x = bounce * pebble_velocity_x;
					pebble.velocity.y = bounce * pebble_velocity_y;
					pebble1.velocity.x = bounce * pebble1_velocity_x;
					pebble1.velocity.y = bounce * pebble1_velocity_y;
					pebble.position.x += pebble_velocity_x * ms;
					pebble.position.y += pebble_velocity_y * ms;
					pebble1.position.x += pebble1_velocity_x * ms;
					pebble1.position.y += pebble1_velocity_y * ms;
				}
			}
			j++;
		}
		i++;
	}
}

void Pebbles::collides_with(const Turtle &turtle, float ms)
{
	for (auto &pebble : m_pebbles)
	{
		vec2 pos = pebble.position;
		vec2 turtle_pos = turtle.get_position();
		int offset_verticle = 50;
		int offset_horizontal = 50;
		// pebble hitting turtle from under
		if (pos.y < turtle_pos.y + offset_verticle && pos.y > turtle_pos.y + offset_verticle - 20 && pos.x > turtle_pos.x - offset_horizontal && pos.x < turtle_pos.x + offset_horizontal)
		{
			// fprintf(stdout, "collision with turtle from under\n");
			pebble.velocity.y = -pebble.velocity.y;
			pebble.position.x += pebble.velocity.x * ms / 1000;
			pebble.position.y += pebble.velocity.y * ms / 1000;
		}
		// Pebble from top
		else if (pos.y > turtle_pos.y - offset_verticle && pos.y < turtle_pos.y - offset_verticle + 20 && pos.x > turtle_pos.x - offset_horizontal && pos.x < turtle_pos.x + offset_horizontal)
		{
			// fprintf(stdout, "collision with turtle from top\n");
			// pebble.angle = -pebble.angle;
			pebble.velocity.y = -pebble.velocity.y;
			pebble.position.x += pebble.velocity.x * ms / 1000;
			pebble.position.y += pebble.velocity.y * ms / 1000;
		}
		// Pebble from left
		else if (pos.x > turtle_pos.x - offset_horizontal && pos.x < turtle_pos.x - offset_horizontal + 20 && pos.y < turtle_pos.y + offset_verticle && pos.y > turtle_pos.y - offset_verticle)
		{
			// fprintf(stdout, "collision with turtle from left\n");
			// if (pebble.angle > 0)
			// 	pebble.angle = PI - pebble.angle;
			// else
			// 	pebble.angle = -PI - pebble.angle;
			pebble.velocity.x = -pebble.velocity.x;
			pebble.velocity.y = -pebble.velocity.y;
			pebble.position.x += pebble.velocity.x * ms / 1000;
			pebble.position.y += pebble.velocity.y * ms / 1000;
		}
		// Pebble from right
		else if (pos.x < turtle_pos.x + offset_horizontal && pos.x > turtle_pos.x && pos.y + offset_horizontal - 20 < turtle_pos.y + offset_verticle && pos.y > turtle_pos.y - offset_verticle)
		{
			// fprintf(stdout, "collision with turtle from right\n");
			// if (pebble.angle > 0)
			// 	pebble.angle = PI - pebble.angle;
			// else
			// 	pebble.angle = -PI - pebble.angle;
			pebble.velocity.x = -pebble.velocity.x;
			pebble.velocity.y = -pebble.velocity.y;
			pebble.position.x += pebble.velocity.x * ms / 1000;
			pebble.position.y += pebble.velocity.y * ms / 1000;
		}
	}
}
void Pebbles::collides_with(const Fish &fish, float ms)
{
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