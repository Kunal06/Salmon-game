// Header
#include "redBox.hpp"

#include <cmath>

Texture RedBox::redBox_texture;

bool RedBox::init()
{
	// Load shared texture
	if (!redBox_texture.is_valid())
	{
		if (!redBox_texture.load_from_file(textures_path("red_box.png")))
		{
			fprintf(stderr, "Failed to load redBox texture!");
			return false;
		}
	}

	// The position corresponds to the center of the texture
	float wr = redBox_texture.width * 0.5f;
	float hr = redBox_texture.height * 0.5f;

	TexturedVertex vertices[4];
	vertices[0].position = {-wr, +hr, -0.02f};
	vertices[0].texcoord = {0.f, 1.f};
	vertices[1].position = {+wr, +hr, -0.02f};
	vertices[1].texcoord = {1.f, 1.f};
	vertices[2].position = {+wr, -hr, -0.02f};
	vertices[2].texcoord = {1.f, 0.f};
	vertices[3].position = {-wr, -hr, -0.02f};
	vertices[3].texcoord = {0.f, 0.f};

	// Counterclockwise as it's the default opengl front winding direction
	uint16_t indices[] = {0, 3, 1, 1, 3, 2};

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;

	collided = false;
	rotate_speed = 0.08;
	motion.position = {900.f, 500.f};
	motion.radians = 0.f;
	motion.speed = 15.f;
	m_is_alive = true;

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture.
	physics.scale = {-0.6f, 0.6f};

	return true;
}

// Releases all graphics resources
// Releases all graphics resources
void RedBox::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

// Called on each frame by World::update()
void RedBox::update(float ms)
{
	const float SALMON_SPEED = 200.f;
	float step = SALMON_SPEED * (ms / 1000);
	vec2 up_vec = {0.f, -10.f};
	vec2 down_vec = {0.f, 10.f};
	vec2 left_vec = {-10.f, 0.f};
	vec2 right_vec = {10.f, 0.f};
	
	if(debug_mode){
		if(motion.speed < 20){
			motion.speed += 0.2;
		}
	}

	if (m_is_alive)
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// UPDATE SALMON POSITION HERE BASED ON KEY PRESSED (World::on_key())
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if (is_up)
		{
			angled_move(1.0);
		}
		else if (is_down)
		{
			angled_move(-1.0);
		}
		else if (is_left)
		{
			rotate(-rotate_speed);
		}
		else if (is_right)
		{
			rotate(rotate_speed);
		}
	}
	else
	{
		// If dead we make it face upwards and sink deep down
		set_rotation(3.1415f);
		move({0.f, step});
	}
}

void RedBox::draw(const mat3 &projection)
{
	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	transform.begin();
	transform.translate(motion.position);
	transform.rotate(motion.radians);
	transform.scale(physics.scale);
	transform.end();

	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Getting uniform locations for glUniform* calls
	GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
	GLint color_uloc = glGetUniformLocation(effect.program, "fcolor");
	GLint projection_uloc = glGetUniformLocation(effect.program, "projection");

	// Setting vertices and indices
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(effect.program, "in_texcoord");
	glEnableVertexAttribArray(in_position_loc);
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)0);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)sizeof(vec3));

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, redBox_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float *)&transform.out);
	float color[] = {1.f, 1.f, 1.f};
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float *)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// HANDLE SALMON - WALL COLLISIONS HERE
// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
// You will want to write new functions from scratch for checking/handling
// redBox - wall collisions.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

bool RedBox::collides_with_wall()
{
	vec2 screen = {1200, 800};
	vec2 pos = motion.position;
	int offset_verticle = 100;
	int offset_horizontal = 120;
	if (debug_mode && collided){
		// rotate_speed = 0.0;
		motion.speed = 2.0;
	}
	if(debug_mode){
		offset_verticle = 120;
		offset_horizontal = 140;
		}
	// top boundary (0,0) to (1200,0)
	if ((pos.y - offset_verticle > 0 && pos.y + offset_verticle < screen.y) && (pos.x - offset_horizontal > 0 && pos.x + offset_horizontal < screen.x ))
	{
		// fprintf(stdout, "right Wall hit\n");
		collided = false;
	}
	else if (pos.y - offset_verticle < 0)
	{
		// fprintf(stdout, "top Wall hit\n");
		reflect(1.0);
		collided = true;
	}
	else if (pos.y + offset_verticle > screen.y)
	{
		// fprintf(stdout, "bottom Wall hit\n");
		reflect(1.0);
		collided = true;
	}
	else if (pos.x - offset_horizontal < 0)
	{
		// fprintf(stdout, "left Wall hit\n");
		reflect(2.0);
		collided = true;
	}
	else if (pos.x + offset_horizontal > screen.x)
	{
		// fprintf(stdout, "right Wall hit\n");
		reflect(2.0);
		collided = true;
	}
	if(debug_mode && collided)
		glfwWaitEventsTimeout(80.0);


	return collided;
}

vec2 RedBox::get_position() const
{
	return motion.position;
}

void RedBox::move(vec2 off)
{
	motion.position.x += off.x;
	motion.position.y += off.y;
	translation_vec = {motion.position.x, motion.position.y};
}

void RedBox::set_rotation(float radians)
{
	motion.radians = radians;
}

void RedBox::rotate(float off)
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

void RedBox::reflect(float value)
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
	float PI = 3.14159;
	if (value == 1.0)
	{
		motion.radians = -motion.radians;
		angled_move(1.0);

	}
	else if (value == 2.0)
	{
		motion.radians = PI - motion.radians;
		angled_move(1.0);
	}
}

void RedBox::angled_move(float off)
{
	motion.position.x += off * motion.speed * 2 * cos(motion.radians);
	motion.position.y += off * motion.speed * 2* sin(motion.radians);
	//translation_vec = {motion.position.x, motion.position.y};
}

vec2 RedBox::get_bounding_box() const
{
	// Returns the local bounding coordinates scaled by the current size of the turtle
	// fabs is to avoid negative scale due to the facing direction.
	return {std::fabs(physics.scale.x), std::fabs(physics.scale.y)};
}

bool RedBox::is_alive() const
{
	return m_is_alive;
}

// Set redBox movement flag (used to keep track of the action and release(f) of the user keys)
void RedBox::set_movement(const std::string &flag)
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

// Get redBox movement flag (used to keep track of the movement)
bool RedBox::get_movement(const std::string &flag)
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

// Called when the redBox collides with a turtle or shark
void RedBox::kill()
{
	m_is_alive = false;
}

void RedBox::set_debug_mode(bool value)
{
	debug_mode = value;
}

void RedBox::set_box_position(vec2 off)
{
	motion.position.x = off.x;
	motion.position.y = off.y;
}

void RedBox::avoid_salmon(vec2 salmon_pos)
{	
	int off = 90;
	box_avoid = 0;
	if(motion.position.y < salmon_pos.y && motion.position.y >= salmon_pos.y - off){
		// move up
		// fprintf(stderr, "Move up\n");
		box_avoid = salmon_pos.y - off - motion.position.y;
	}
	else if(motion.position.y >= salmon_pos.y && motion.position.y <= salmon_pos.y + off)
	{
		// move down
		// fprintf(stderr, "Move down\n");
		box_avoid = salmon_pos.y + off - motion.position.y ;
	}
	else 
	{
		box_avoid = 0;
	}
	// fprintf(stderr, "Box avoid - %d\n", box_avoid);
	motion.position.y += box_avoid;
	
}
