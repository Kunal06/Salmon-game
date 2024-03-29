#pragma once

#include "common.hpp"


class Water : public Entity
{
public:
	int view_port[4];

	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	// Renders the water
	void draw(const mat3& projection)override;

	// Salmon dead time getters and setters
	void set_salmon_dead();
	void reset_salmon_dead_time();
	float get_salmon_dead_time() const;
	bool draw_rect_init();
	void draw_rect(int debug_mode);


private:
	// When salmon is alive, the time is set to -1
	float m_dead_time;
};
