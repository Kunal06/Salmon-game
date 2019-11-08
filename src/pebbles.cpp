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
		float PI = 3.14159;

		vec2 a = {0.f, 9.81f};
		float dt = (ms) / 1000;

		// HORIZONTAL
		// v = u + at, a=0
		// pebble.velocity.x = pebble.velocity.x + acceleration.x * dt;
		// // delta x = v*t
		// float step_x = pebble.velocity.x * dt;
		// pebble.position.x += step_x * cos(pebble.angle) ;

		// HORIZONTAL
		float step_x = pebble.velocity.x * dt;
		//pebble.velocity.x -= step_x;
		pebble.position.x += step_x * cos(pebble.angle);

		// VERTICAL
		float delta_y = 800 - pebble.position.y ;
		float a_y = a.y;
		float Vy_i = pebble.velocity.y * sin(pebble.angle);
		if (pebble.angle == 0 )
			Vy_i = pebble.velocity.y * sin(0.5 + pebble.angle);
		
			// y = ut + 1/2 a t^2 , t= dt
		float step_y = Vy_i *dt + 1/2 * a.y;
		float off = 1.0;

		if (pebble.angle >= PI)
			off = -1.0;
		pebble.velocity.y += off * step_y;
		//fprintf(stderr, "\nPebble - update Velocity - %f \n ", pebble.velocity.y);
		pebble.position.y += off * step_y ;
	}
	//collides_with((ms) / 1000);
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
		angle = fmod (angle,2*PI);
		float MAX = angle + PI / 2;
		float MIN = angle - PI / 2;
		if(angle >= -PI/2 && angle < PI/2){
			MIN =0.5;
		}
		float direction = (float(rand()) / (float(RAND_MAX) / (MAX - MIN))) + MIN;
		peb.radius = 10;
		peb.velocity.x = velocity;
		peb.velocity.y = 200.f;
		peb.angle =direction;
		peb.mass = 1.f;
		// if((angle < PI && angle > PI/2) || (angle > 1.57 && angle < 3.14))
		// else
		// 	peb.angle = angle;

		m_pebbles.emplace_back(peb);
		COOLDOWN = 4;
	}
	else
	{
		COOLDOWN--;
	}
	// fprintf(stderr, "pebble spawned pebblie file \n");
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
					//fprintf(stderr, "\nPebble - Collides - %d - with - %d \n ", i, j);
					//accurate collision points
					float collisionPointX =
						((p_pos.x * pebble1.radius) + (p1_pos.x * pebble.radius)) / (pebble.radius + pebble1.radius);
					float collisionPointY =
						((p_pos.y * pebble1.radius) + (p1_pos.y * pebble.radius)) / (pebble.radius + pebble1.radius);
					// fprintf(stderr, "\n before Pebble - Collides - %f - with - %f \n ", pebble.velocity.x, pebble.velocity.y);
					// pebble.velocity.x = (pebble.velocity.x * (pebble.mass - pebble1.mass) + (2 * pebble1.mass * pebble1.velocity.x)) / (pebble.mass + pebble1.mass);
					// pebble.velocity.y = (pebble.velocity.y * (pebble.mass - pebble1.mass) + (2 * pebble1.mass * pebble1.velocity.y)) / (pebble.mass + pebble1.mass);
					// pebble1.velocity.x = (pebble1.velocity.x * (pebble1.mass - pebble.mass) + (2 * pebble.mass * pebble.velocity.x)) / (pebble.mass + pebble1.mass);
					// pebble1.velocity.y = (pebble1.velocity.y * (pebble1.mass - pebble.mass) + (2 * pebble.mass * pebble.velocity.y)) / (pebble.mass + pebble1.mass);
					// fprintf(stderr, "\nAfter Pebble - Collides - %f - with - %f \n ", pebble.velocity.x, pebble.velocity.y);
					// pebble.position.x += pebble.velocity.x * ms;
					// pebble1.position.x += pebble1.velocity.x* ms;
					// pebble.velocity.x += ((pebble.velocity.x * (pebble.mass - pebble1.mass) + (2 * pebble1.mass * pebble1.velocity.x)) / (pebble.mass + pebble1.mass))/ms - pebble1.velocity.x ;
					// pebble.velocity.y += ((pebble.velocity.y * (pebble.mass - pebble1.mass) + (2 * pebble1.mass * pebble1.velocity.y)) / (pebble.mass + pebble1.mass))/ms;
					// pebble1.velocity.x += ((pebble1.velocity.x * (pebble1.mass - pebble.mass) + (2 * pebble.mass * pebble.velocity.x)) / (pebble.mass + pebble1.mass))/ms;
					// pebble1.velocity.y += ((pebble1.velocity.y * (pebble1.mass - pebble.mass) + (2 * pebble.mass * pebble.velocity.y)) / (pebble.mass + pebble1.mass))/ms;
					//fprintf(stderr, "\nAfter Pebble - Collides - %f - with - %f \n ", pebble.velocity.x, pebble.velocity.y);
					// pebble.velocity.x = (pebble.velocity.x - ((pebble.velocity.x - pebble1.velocity.x)*(pebble.position.x - pebble1.position.x))/pow(fabs(pebble.position.x - pebble1.position.x),2) * (pebble.position.x - pebble1.position.x))*ms;
					// pebble.velocity.y = pebble.velocity.y - ((pebble.velocity.y - pebble1.velocity.y)*(pebble.position.y - pebble1.position.y))/pow(fabs(pebble.position.y - pebble1.position.y),2) * (pebble.position.y - pebble1.position.y);
					

					// pebble.position.x+= pebble.velocity.x;
				}
			}
			j++;
		}
		i++;
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
		vec2 pos = pebble.position;
		vec2 turtle_pos = turtle.get_position();
		// int offset_verticle = 50;
		// int offset_horizontal = 50;
		// float PI = 3.14159;
		// // pebble hitting turtle from under
		// if (pos.y  < turtle_pos.y + offset_verticle && pos.y > turtle_pos.y && pos.x > turtle_pos.x - offset_horizontal && pos.x < turtle_pos.x + offset_horizontal)
		// {
		// 	// fprintf(stdout, "right Wall hit\n");
		// 	pebble.angle = - pebble.angle;
		// }
		// else if (pos.y > turtle_pos.y - offset_verticle && pos.y < turtle_pos.y && pos.x > turtle_pos.x - offset_horizontal && pos.x < turtle_pos.x + offset_horizontal)
		// {
		// 	// fprintf(stdout, "top Wall hit\n");
		// 	pebble.angle = - pebble.angle;
		// }
		// else if (pos.x > turtle_pos.x - offset_horizontal && pos.x < turtle_pos.x && pos.y < turtle_pos.y + offset_verticle  && pos.y > turtle_pos.y - offset_verticle)
		// {
		// 	// fprintf(stdout, "bottom Wall hit\n");
		// 	pebble.angle = PI - pebble.angle;
		// 	pebble.position.x +=  pebble.velocity.x * cos(pebble.angle);
		// 	pebble.position.y +=  pebble.velocity.x * sin(pebble.angle);

		// }
		// else if (pos.x < turtle_pos.x + offset_horizontal && pos.x > turtle_pos.x && pos.y < turtle_pos.y + offset_verticle  && pos.y > turtle_pos.y - offset_verticle)
		// {
		// 	// fprintf(stdout, "bottom Wall hit\n");
		// 	pebble.angle = PI - pebble.angle;
		// 	pebble.position.x +=  pebble.velocity.x * cos(pebble.angle);
		// 	pebble.position.y +=  pebble.velocity.x * sin(pebble.angle);
		// }

		float dx = pebble.position.x - turtle.get_position().x;
		float dy = pebble.position.y - turtle.get_position().y;
		float d_sq = dx * dx + dy * dy;
		float other_r = std::max(turtle.get_bounding_box().x, turtle.get_bounding_box().y);
		float my_r = pebble.radius;
		float r = std::max(other_r, my_r);
		r *= 0.6f;
		if (d_sq < r * r)
		{
			if (turtle.get_bounding_box().x < turtle.get_bounding_box().y)
				pebble.angle =  3.14 - pebble.angle;
			else
				pebble.angle =  - pebble.angle;
			//fprintf(stderr, "\nPebble - Collides - %f \n ", pebble.angle);
		}
	}
}

void Pebbles::collides_with(const Fish &fish)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PEBBLE COLLISIONS HERE
	// You will need to write additional functions from scratch.
	// Make sure to handle both collisions between pebbles
	// and collisions between pebbles and salmon/fish/turtles.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	for (auto &pebble : m_pebbles)
	{
		float dx = pebble.position.x - fish.get_position().x;
		float dy = pebble.position.y - fish.get_position().y;
		float d_sq = dx * dx + dy * dy;
		float other_r = std::max(fish.get_bounding_box().x, fish.get_bounding_box().y);
		float my_r = pebble.radius;
		float r = std::max(other_r, my_r);
		r *= 0.6f;
		if (d_sq < r * r)
		{
			//pebble.angle = 3.14 - pebble.angle;
			//fprintf(stderr, "\nPebble - Collides - %f \n ", pebble.angle);
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