#pragma once

#include "common.hpp"

// Box enemy
class Box : public Entity
{
	// Shared between all boxs, no need to load one for each instance
	static Texture box_texture;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	// Update box position based on direction
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the box
	void draw(const mat3 &projection) override;


	bool collides_with_wall();
	// Returns the current box position
	vec2 get_position() const;

	vec2 get_bounding_box() const;

	// Moves the box's position by the specified offset
	void move(vec2 off);

	// Set box rotation in radians
	void set_rotation(float radians);

	// Set box rotation in radians
	void rotate(float off);
	void reflect(float value);
	void angled_move(float off);
	// True if the box is alive
	bool is_alive() const;

	// Kills the box, changing its alive state and triggering on death events
	void kill();

	// Set movement flag of Box(keep track of the movement)
	void set_movement(const std::string &flag);

	// Get box movement flag (keep track of the movement )
	bool get_movement(const std::string &flag);

	void set_debug_mode(bool value);

	void set_box_position(vec2 off);

private:
	bool m_is_alive = false;
	bool advanced_m = true;
	bool random_m = false;
	bool is_up = false;
	bool is_down = false;
	bool is_left = false;
	bool is_right = false;
	bool debug_mode = false;
	bool collided = false;
	float rotate_speed = 0.08;

	vec2 translation_vec;
};
