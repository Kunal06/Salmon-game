#pragma once

#include "common.hpp"

// RedBox enemy
class RedBox : public Entity
{
	// Shared between all redBoxes, no need to load one for each instance
	static Texture redBox_texture;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	// Update redBox position based on direction
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the redBox
	void draw(const mat3 &projection) override;


	bool collides_with_wall();
	// Returns the current redBox position
	vec2 get_position() const;

	vec2 get_bounding_box() const;

	// Moves the redBox's position by the specified offset
	void move(vec2 off);

	// Set redBox rotation in radians
	void set_rotation(float radians);

	// Set redBox rotation in radians
	void rotate(float off);
	void reflect(float value);
	void angled_move(float off);
	// True if the redBox is alive
	bool is_alive() const;

	// Kills the redBox, changing its alive state and triggering on death events
	void kill();

	// Set movement flag of RedBox(keep track of the movement)
	void set_movement(const std::string &flag);

	// Get redBox movement flag (keep track of the movement )
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
