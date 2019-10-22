#pragma once

#include "common.hpp"

class Turtle;
class Fish;
class Shark;

class Salmon : public Entity
{
public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	// Update salmon position based on direction
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the salmon
	void draw(const mat3 &projection) override;

	// Collision routines for turtles and fish and shark
	bool collides_with(const Turtle &turtle);
	bool collides_with(const Fish &fish);
	bool collides_with(const Shark &shark);

	// Returns the current salmon position
	vec2 get_position() const;

	// Moves the salmon's position by the specified offset
	void move(vec2 off);

	// Set salmon rotation in radians
	void set_rotation(float radians);

	// True if the salmon is alive
	bool is_alive() const;

	// Kills the salmon, changing its alive state and triggering on death events
	void kill();

	// Called when the salmon collides with a fish, starts lighting up the salmon
	void light_up();

	// Set movement flag of Salmon(keep track of the movement)
	void set_movement(const std::string &flag);

	// Get salmon movement flag (keep track of the movement )
	bool get_movement(const std::string &flag);

private:
	float m_light_up_countdown_ms; // Used to keep track for how long the salmon should be lit up
	bool m_is_alive;			   // True if the salmon is alive
};
