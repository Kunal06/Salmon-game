// Header
#include "world.hpp"

// stlib
#include <string.h>
#include <cassert>
#include <sstream>

// Same as static in c, local to compilation unit
namespace
{
int MAX_TURTLES = 15;
const size_t MAX_FISH = 5;
const size_t MAX_PEBBLES = 5;
const size_t PEBBLES_DELAY_MS = 500;
const size_t TURTLE_DELAY_MS = 2000;
const size_t FISH_DELAY_MS = 5000;
const size_t MAX_SHARKS = 5;
const size_t SHARK_DELAY_MS = 12000;
bool advanced = false;
bool debug_mode = false;
bool collision_value = false;
bool follow_mode = false;
bool spawn_pebbles = false;
int X_frames = 0;
int FRAME_LIMIT = 0;

namespace
{
void glfw_err_cb(int error, const char *desc)
{
	fprintf(stderr, "%d: %s", error, desc);
}
} // namespace
} // namespace

World::World() : m_points(0),
				 m_next_turtle_spawn(0.f),
				 m_next_fish_spawn(0.f),
				 // Shark ADDED
				 m_next_shark_spawn(0.f)
{
	// Seeding rng with random device
	m_rng = std::default_random_engine(std::random_device()());
}

World::~World()
{
}

// World initialization
bool World::init(vec2 screen)
{
	//-------------------------------------------------------------------------
	// GLFW / OGL Initialization
	// Core Opengl 3.
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);
	m_window = glfwCreateWindow((int)screen.x, (int)screen.y, "Salmon Game Assignment", nullptr, nullptr);
	if (m_window == nullptr)
		return false;

	glfwMakeContextCurrent(m_window);
	glfwSwapInterval(1); // vsync

	// Load OpenGL function pointers
	gl3w_init();

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(m_window, this);
	auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3) { ((World *)glfwGetWindowUserPointer(wnd))->on_key(wnd, _0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1) { ((World *)glfwGetWindowUserPointer(wnd))->on_mouse_move(wnd, _0, _1); };
	glfwSetKeyCallback(m_window, key_redirect);
	glfwSetCursorPosCallback(m_window, cursor_pos_redirect);

	// Create a frame buffer
	m_frame_buffer = 0;
	glGenFramebuffers(1, &m_frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

	// For some high DPI displays (ex. Retina Display on Macbooks)
	// https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
	int fb_width, fb_height;
	glfwGetFramebufferSize(m_window, &fb_width, &fb_height);
	m_screen_scale = static_cast<float>(fb_width) / screen.x;

	// Initialize the screen texture
	m_screen_tex.create_from_screen(m_window);

	//-------------------------------------------------------------------------
	// Loading music and sounds
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return false;
	}

	m_background_music = Mix_LoadMUS(audio_path("music.wav"));
	m_salmon_dead_sound = Mix_LoadWAV(audio_path("salmon_dead.wav"));
	m_salmon_eat_sound = Mix_LoadWAV(audio_path("salmon_eat.wav"));

	if (m_background_music == nullptr || m_salmon_dead_sound == nullptr || m_salmon_eat_sound == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
				audio_path("music.wav"),
				audio_path("salmon_dead.wav"),
				audio_path("salmon_eat.wav"));
		return false;
	}

	// Playing background music indefinitely
	// Mix_PlayMusic(m_background_music, -1);

	fprintf(stderr, "Loaded music\n");

	m_current_speed = 1.f;

	return m_salmon.init() && m_water.init() && m_pebbles_emitter.init() && m_water.draw_rect_init() && m_box.init() && m_redbox.init();
}

// Releases all the associated resources
void World::destroy()
{
	glDeleteFramebuffers(1, &m_frame_buffer);

	if (m_background_music != nullptr)
		Mix_FreeMusic(m_background_music);
	if (m_salmon_dead_sound != nullptr)
		Mix_FreeChunk(m_salmon_dead_sound);
	if (m_salmon_eat_sound != nullptr)
		Mix_FreeChunk(m_salmon_eat_sound);

	Mix_CloseAudio();

	m_salmon.destroy();
	m_box.destroy();
	m_pebbles_emitter.destroy();
	for (auto &turtle : m_turtles)
		turtle.destroy();
	for (auto &fish : m_fish)
		fish.destroy();
	if (advanced)
	{
		// Shark ADDED
		for (auto &shark : m_sharks)
			shark.destroy();
	}
	m_turtles.clear();
	m_fish.clear();
	if (advanced)
	{
		// Shark ADDED
		m_sharks.clear();
	}
	glfwDestroyWindow(m_window);
}

// Update our game world
bool World::update(float elapsed_ms)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	vec2 screen = {(float)w / m_screen_scale, (float)h / m_screen_scale};

	// Checking Salmon - Turtle collisions
	for (const auto &turtle : m_turtles)
	{
		if (m_salmon.collides_with(turtle))
		{
			if (m_salmon.is_alive())
			{
				// Mix_PlayChannel(-1, m_salmon_dead_sound, 0);
				m_water.set_salmon_dead();
			}
			m_salmon.kill();
			m_box.kill();
			break;
		}
		m_pebbles_emitter.collides_with(turtle);
	}

	// Checking Salmon - Fish collisions
	auto fish_it = m_fish.begin();
	while (fish_it != m_fish.end())
	{
		if (m_salmon.is_alive() && m_salmon.collides_with(*fish_it))
		{
			fish_it = m_fish.erase(fish_it);
			m_salmon.light_up();
			// Mix_PlayChannel(-1, m_salmon_eat_sound, 0);
			++m_points;
		}
		else
			++fish_it;
		
		//m_pebbles_emitter.collides_with(*fish_it);
	}
	if (advanced)
	{
		// Checking Salmon - Shark collisions -- Shark Added
		for (const auto &shark : m_sharks)
		{
			if (m_salmon.collides_with(shark))
			{
				if (m_salmon.is_alive())
				{
					// Mix_PlayChannel(-1, m_salmon_dead_sound, 0);
					m_water.set_salmon_dead();
				}
				m_salmon.kill();
				m_box.kill();
				break;
			}
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE SALMON - WALL COLLISIONS HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (m_salmon.is_alive())
	{
		collision_value = m_salmon.collides_with_wall();
		if (debug_mode)
		{
			if (collision_value)
			{
				m_current_speed = 0.f;
			}
			if (m_current_speed < 1.f)
			{
				m_current_speed += 0.1;
			}
		}
	}
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PEBBLE COLLISIONS HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Updating all entities, making the turtle and fish and shark
	// faster based on current.
	// In a pure ECS engine we would classify entities by their bitmap tags during the update loop
	// rather than by their class.
	m_pebbles_emitter.update(elapsed_ms);
	m_salmon.update(elapsed_ms);
	vec2 salmon_pos = m_salmon.get_position();
	// m_box.update(elapsed_ms);
	m_box.set_box_position(salmon_pos);
	for (auto &turtle : m_turtles)
		turtle.update(elapsed_ms * m_current_speed);
	for (auto &fish : m_fish)
	{
		fish.update(elapsed_ms * m_current_speed);
		fish.set_advanced(advanced);
	}
	if (advanced)
	{
		// Shark Added
		for (auto &shark : m_sharks)
			shark.update(elapsed_ms * m_current_speed);
	}
	// fprintf(stderr, "Salmon y position - %f\n", salmon_pos.y);
	// Fish avoid Salmon
	// if (X_frames == FRAME_LIMIT)
	// {
	for (auto &fish : m_fish)
	{
		if (m_salmon.avoid(fish))
		{

			vec2 fish_pos = fish.get_position();
			fish.avoid_salmon(salmon_pos);
		}
	}
	//}

	// Turtle follow
	if (follow_mode)
	{
		MAX_TURTLES = 5;
		for (auto &turtle : m_turtles)
		{
			turtle.set_follow_mode(true);
			vec2 turtle_pos = turtle.get_position();

			// direction from turtle to salmon
			float rotate = (GLfloat)atan2(turtle_pos.y - salmon_pos.y, turtle_pos.x - salmon_pos.x);
			turtle.rotate(rotate);
		}
	}
	else
	{
		MAX_TURTLES = 15;
		for (auto &turtle : m_turtles)
		{
			turtle.set_follow_mode(false);
			vec2 turtle_pos = turtle.get_position();
			turtle.rotate(0.f);
		}
	}
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PEBBLE SPAWN/UPDATES HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// Spawning new Pebbles
	if (spawn_pebbles)
	{
		//fprintf(stderr, "spawn pebble called");
		m_pebbles_emitter.spawn_pebble(m_salmon.get_position(), m_salmon.get_rotation());
	}
	// Removing out of screen turtles
	auto turtle_it = m_turtles.begin();
	while (turtle_it != m_turtles.end())
	{
		float w = turtle_it->get_bounding_box().x / 2;
		if (turtle_it->get_position().x + w < 0.f)
		{
			turtle_it = m_turtles.erase(turtle_it);
			continue;
		}

		++turtle_it;
	}

	// Removing out of screen fish
	fish_it = m_fish.begin();
	while (fish_it != m_fish.end())
	{
		float w = fish_it->get_bounding_box().x / 2;
		if (fish_it->get_position().x + w < 0.f)
		{
			fish_it = m_fish.erase(fish_it);
			continue;
		}

		++fish_it;
	}
	if (advanced)
	{
		// SHark Added
		// Removing out of screen sharks
		auto shark_it = m_sharks.begin();
		while (shark_it != m_sharks.end())
		{
			float w = shark_it->get_bounding_box().x / 2;
			if (shark_it->get_position().x + w < 0.f)
			{
				shark_it = m_sharks.erase(shark_it);
				continue;
			}

			++shark_it;
		}
	}

	// Spawning new turtles
	m_next_turtle_spawn -= elapsed_ms * m_current_speed;
	if (m_turtles.size() <= MAX_TURTLES && m_next_turtle_spawn < 0.f)
	{
		if (!spawn_turtle())
			return false;

		Turtle &new_turtle = m_turtles.back();
		if (advanced)
		{
			new_turtle.set_advanced(true);
		}
		else
		{
			new_turtle.set_advanced(false);
		}
		// Setting random initial position
		new_turtle.set_position({screen.x + 150, 50 + m_dist(m_rng) * (screen.y - 100)});

		// Next spawn
		m_next_turtle_spawn = (TURTLE_DELAY_MS / 2) + m_dist(m_rng) * (TURTLE_DELAY_MS / 2);
	}

	// Spawning new fish
	m_next_fish_spawn -= elapsed_ms * m_current_speed;
	if (m_fish.size() <= MAX_FISH && m_next_fish_spawn < 0.f)
	{
		if (!spawn_fish())
			return false;
		Fish &new_fish = m_fish.back();

		new_fish.set_position({screen.x + 150, 50 + m_dist(m_rng) * (screen.y - 100)});

		m_next_fish_spawn = (FISH_DELAY_MS / 2) + m_dist(m_rng) * (FISH_DELAY_MS / 2);
	}
	if (advanced)
	{
		// SHark Added
		// Spawning new sharks
		m_next_shark_spawn -= elapsed_ms * m_current_speed;
		if (m_sharks.size() <= MAX_SHARKS && m_next_shark_spawn < 0.f)
		{
			if (!spawn_shark())
				return false;

			Shark &new_shark = m_sharks.back();

			// Setting random initial position
			new_shark.set_position({screen.x + 150, 50 + m_dist(m_rng) * (screen.y - 100)});

			// Next spawn
			m_next_shark_spawn = (SHARK_DELAY_MS / 2) + m_dist(m_rng) * (SHARK_DELAY_MS / 2);
		}
	}
	// If salmon is dead, restart the game after the fading animation
	if (!m_salmon.is_alive() &&
		m_water.get_salmon_dead_time() > 5)
	{
		follow_mode = false;
		debug_mode = false;
		advanced = false;
		m_salmon.destroy();
		m_box.destroy();
		m_salmon.init();
		m_box.init();
		m_pebbles_emitter.destroy();
		m_pebbles_emitter.init();
		m_turtles.clear();
		m_fish.clear();
		if (advanced)
		{
			m_sharks.clear();
		}
		m_water.reset_salmon_dead_time();
		m_current_speed = 1.f;
	}
	return true;
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void World::draw()
{
	// Clearing error buffer
	gl_flush_errors();

	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);

	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << m_points;
	glfwSetWindowTitle(m_window, title_ss.str().c_str());

	/////////////////////////////////////
	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	const float clear_color[3] = {0.3f, 0.3f, 0.8f};
	glClearColor(clear_color[0], clear_color[1], clear_color[2], 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Fake projection matrix, scales with respect to window coordinates
	// PS: 1.f / w in [1][1] is correct.. do you know why ? (:
	float left = 0.f;						  // *-0.5;
	float top = 0.f;						  // (float)h * -0.5;
	float right = (float)w / m_screen_scale;  // *0.5;
	float bottom = (float)h / m_screen_scale; // *0.5;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	mat3 projection_2D{{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// DRAW DEBUG INFO HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will want to create your own data structures for passing in
	// relevant information to your debug draw call.
	// The shaders coloured.vs.glsl and coloured.fs.glsl should be helpful.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	if (debug_mode)
	{
		int off = 120;
		m_water.draw_rect(1);
		m_salmon.set_debug_mode(true);
		m_box.draw(projection_2D);
		if (m_salmon.collides_with_wall())
		{
			// fprintf(stderr, "DRAW collision point");
			m_redbox.set_box_position(m_salmon.get_position());
			m_redbox.draw(projection_2D);
		}
		for (auto &fish : m_fish)
		{
			vec2 fish_pos = fish.get_position();
			while (fish_pos.x > 90)
			{
				fish_pos.x -= off;
				vec2 box_position = {fish_pos.x, fish_pos.y};
				vec2 salmon_pos = m_salmon.get_position();
				// check if box collides with salmon
				m_redbox.set_box_position(box_position);
				// if (X_frames == FRAME_LIMIT)
				// {
				if ((fish_pos.x - salmon_pos.x) < 160)
				{
					m_redbox.avoid_salmon(m_salmon.get_position());
				}
				X_frames = 0;
				//  }
				m_redbox.draw(projection_2D);
			}
		}

		// m_redbox.draw(projection_2D);
	}
	else
	{
		m_water.draw_rect(0);
		m_salmon.set_debug_mode(false);
	}
	// if (X_frames != FRAME_LIMIT)
	// 	X_frames++;
	// Drawing entities
	for (auto &turtle : m_turtles)
		turtle.draw(projection_2D);
	for (auto &fish : m_fish)
		fish.draw(projection_2D);
	if (advanced)
	{
		// Shark Added
		for (auto &shark : m_sharks)
			shark.draw(projection_2D);
	}
	m_pebbles_emitter.draw(projection_2D);
	m_salmon.draw(projection_2D);
	/////////////////////
	// Truely render to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(0, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_screen_tex.id);

	m_water.draw(projection_2D);

	//////////////////
	// Presenting
	glfwSwapBuffers(m_window);
}

// Should the game be over ?
bool World::is_over() const
{
	return glfwWindowShouldClose(m_window);
}

// Creates a new turtle and if successfull adds it to the list of turtles
bool World::spawn_turtle()
{
	Turtle turtle;
	if (turtle.init())
	{
		m_turtles.emplace_back(turtle);
		return true;
	}
	fprintf(stderr, "Failed to spawn turtle");
	return false;
}

// Creates a new fish and if successfull adds it to the list of fish
bool World::spawn_fish()
{
	Fish fish;
	if (fish.init())
	{
		m_fish.emplace_back(fish);
		return true;
	}
	fprintf(stderr, "Failed to spawn fish");
	return false;
}

// Creates a new shark and if successfull adds it to the list of sharks
bool World::spawn_shark()
{
	Shark shark;
	if (shark.init())
	{
		m_sharks.emplace_back(shark);
		return true;
	}
	fprintf(stderr, "Failed to spawn shark");
	return false;
}

// On key callback
void World::on_key(GLFWwindow *, int key, int, int action, int mod)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE SALMON MOVEMENT HERE
	// key is of 'type' GLFW_KEY_
	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// Turns the game advanced mode on
	if (action == GLFW_PRESS && key == GLFW_KEY_A)
	{
		advanced = !advanced;
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_D)
	{
		if (m_salmon.is_alive())
			debug_mode = !debug_mode;
		// fprintf(stderr, "Debug mode - %d", debug_mode );
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_F)
	{
		follow_mode = !follow_mode;
		//  fprintf(stderr, "Follow mode - %d", follow_mode );
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_SPACE)
	{
		spawn_pebbles = true;
	}
	if (action == GLFW_RELEASE && key == GLFW_KEY_SPACE)
	{
		spawn_pebbles = false;
	}
	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		int w, h;
		glfwGetWindowSize(m_window, &w, &h);
		m_salmon.destroy();
		m_box.destroy();
		m_salmon.init();
		m_box.init();
		m_pebbles_emitter.destroy();
		m_pebbles_emitter.init();
		m_turtles.clear();
		m_fish.clear();
		// SHark Added
		m_sharks.clear();
		m_water.reset_salmon_dead_time();
		m_current_speed = 1.f;
		debug_mode = false;
	}
	// Moving Down
	if (action == GLFW_PRESS && key == GLFW_KEY_DOWN)
	{
		m_salmon.set_movement("down");
		m_box.set_movement("down");
	}
	else if (action == GLFW_RELEASE && key == GLFW_KEY_DOWN)
	{
		m_salmon.set_movement("downf");
		m_box.set_movement("downf");
	}

	// Moving Up
	if (action == GLFW_PRESS && key == GLFW_KEY_UP)
	{
		m_salmon.set_movement("up");
		m_box.set_movement("up");
	}
	else if (action == GLFW_RELEASE && key == GLFW_KEY_UP)
	{
		m_salmon.set_movement("upf");
		m_box.set_movement("upf");
	}

	// Moving Left
	if (action == GLFW_PRESS && key == GLFW_KEY_LEFT)
	{

		m_salmon.set_movement("left");
		m_box.set_movement("left");
		// int w, h;
		// vec2 salmon_pos = m_salmon.get_position();
		// vec2 screen = {(float)w / m_screen_scale, (float)h / m_screen_scale};

		// fprintf(stderr, "xposition - %f\n", screen.x);
		// fprintf(stderr, "yposition - %f\n", screen.y);
		// float rotate = (GLfloat)atan2(screen.y - salmon_pos.y, screen.x - salmon_pos.x); //ROTATE TO MOUSE
		// m_salmon.set_rotation(rotate);
		// m_salmon.rotate();
	}
	else if (action == GLFW_RELEASE && key == GLFW_KEY_LEFT)
	{
		m_salmon.set_movement("leftf");
		m_box.set_movement("leftf");
	}

	// Moving Right
	if (action == GLFW_PRESS && key == GLFW_KEY_RIGHT)
	{
		m_salmon.set_movement("right");
		m_box.set_movement("right");
	}
	else if (action == GLFW_RELEASE && key == GLFW_KEY_RIGHT)
	{
		m_salmon.set_movement("rightf");
		m_box.set_movement("rightf");
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA)
		m_current_speed -= 0.1f;
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD)
		m_current_speed += 0.1f;

	m_current_speed = fmax(0.f, m_current_speed);

	if (action == GLFW_RELEASE && key == GLFW_KEY_MINUS)
		FRAME_LIMIT -= 1;
	if (action == GLFW_RELEASE && key == GLFW_KEY_EQUAL)
		FRAME_LIMIT += 1;
	FRAME_LIMIT = fmax(0, FRAME_LIMIT);
}

void World::on_mouse_move(GLFWwindow *window, double xpos, double ypos)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE SALMON ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the salmon's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	vec2 salmon_pos = m_salmon.get_position();
	// fprintf(stderr, "xposition - %f\n", xpos);
	// fprintf(stderr, " yposition - %f\n", ypos);
	float angle = atan2(ypos - salmon_pos.y, xpos - salmon_pos.x);
	float rotate = (GLfloat)atan2(ypos - salmon_pos.y, xpos - salmon_pos.x); //ROTATE TO MOUSE

	// m_salmon.set_rotation(rotate);
}
