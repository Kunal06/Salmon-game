#pragma once

#include "common.hpp"

// Salmon enemy
class Shark : public Entity
{
	// Shared between all sharks, no need to load one for each instance
	static Texture shark_texture;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all the associated resources
	void destroy();

	// Update shark due to current
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the salmon
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection) override;

	// Returns the current shark position
	vec2 get_position()const;

	// Sets the new shark position
	void set_position(vec2 position);

	// Returns the shark' bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box() const;
};
