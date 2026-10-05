/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
 * SDL3 port: 2026, bTanks Remastered
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#include "sdlx.h"

#include "sdlx/joystick.h"
#include "sdlx/sdl_ex.h"

using namespace sdlx;

const int Joystick::getCount() {
	int count = 0;
	SDL_JoystickID *joys = SDL_GetJoysticks(&count);
	if (joys == NULL)
		throw_sdl(("SDL_GetJoysticks"));
	SDL_free(joys);
	return count;
}

const std::string Joystick::getName(const int idx) {
	int count = 0;
	SDL_JoystickID *joys = SDL_GetJoysticks(&count);
	if (joys == NULL)
		throw_sdl(("SDL_GetJoysticks"));
	std::string name;
	if (idx >= 0 && idx < count) {
		const char *name_str = SDL_GetJoystickNameForID(joys[idx]);
		if (name_str != NULL) {
			name = name_str;
			mrt::trim(name);
		}
	}
	SDL_free(joys);
	return name;
}

void Joystick::sendEvents(const bool enable) {
	SDL_SetJoystickEventsEnabled(enable);
}


Joystick::Joystick() : _joy(NULL) {}

void Joystick::open(const int idx) {
	close();
	int count = 0;
	SDL_JoystickID *joys = SDL_GetJoysticks(&count);
	if (joys == NULL)
		throw_sdl(("SDL_GetJoysticks"));
	SDL_JoystickID id = (idx >= 0 && idx < count)? joys[idx] : 0;
	SDL_free(joys);
	if (id == 0)
		throw_ex(("joystick index %d is out of range", idx));
	_joy = SDL_OpenJoystick(id);
	if (_joy == NULL)
		throw_sdl(("SDL_OpenJoystick(%d)", idx));
}

Joystick::Joystick(const int idx) : _joy (NULL) {
	open(idx);
}

const bool Joystick::opened() const {
	return _joy != NULL;
}

Sint16 Joystick::get_axis(const int idx) const {
	if (_joy == NULL)
		throw_ex(("get_axis(%d) on uninitialized joystick", idx));
	return SDL_GetJoystickAxis(_joy, idx);
}

const bool Joystick::get_button(const int idx) const {
	if (_joy == NULL)
		throw_ex(("get_button(%d) on uninitialized joystick", idx));
	return SDL_GetJoystickButton(_joy, idx) != 0;
}

const int Joystick::get_hat(const int idx) const {
	if (_joy == NULL)
		throw_ex(("get_hat(%d) on uninitialized joystick", idx));
	return SDL_GetJoystickHat(_joy, idx);
}

void Joystick::get_ball(const int idx, int &dx, int &dy) const {
	if (_joy == NULL)
		throw_ex(("get_ball(%d) on uninitialized joystick", idx));
	if (!SDL_GetJoystickBall(_joy, idx, &dx, &dy))
		throw_sdl(("SDL_GetJoystickBall(%d)", idx));
}


const int Joystick::get_axis_num() const {
	if (_joy == NULL)
		throw_ex(("get_axis_num() on uninitialized joystick"));
	return SDL_GetNumJoystickAxes(_joy);
}

const int Joystick::get_buttons_num() const {
	if (_joy == NULL)
		throw_ex(("get_buttons_num() on uninitialized joystick"));
	return SDL_GetNumJoystickButtons(_joy);
}

const int Joystick::get_balls_num() const {
	if (_joy == NULL)
		throw_ex(("get_balls_num() on uninitialized joystick"));
	return SDL_GetNumJoystickBalls(_joy);
}

const int Joystick::get_hats_num() const {
	if (_joy == NULL)
		throw_ex(("get_hats_num() on uninitialized joystick"));
	return SDL_GetNumJoystickHats(_joy);
}

void Joystick::close() {
	if (_joy == NULL)
		return;

	SDL_CloseJoystick(_joy);
	_joy = NULL;
}

Joystick::~Joystick() {
	close();
}
