/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
 * SDL3 port: 2026, bTanks Remastered
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
*/


#include "sdlx/system.h"
#include "sdlx/sdlx.h"
#include "sdlx/sdl_ex.h"
#include <stdlib.h>

#include "mrt/logger.h"

using namespace sdlx;

const bool System::accelerated_gl(const bool windowed) {
	//renderer-based drawing does not need the legacy GL probe:
	//SDL picks the best available backend (Metal/Vulkan/GL) itself
	return true;
}

void System::init(int system) {
	LOG_DEBUG(("calling SDL_init('%08x')", (unsigned)system));
	if (!SDL_Init(system))
		throw_sdl(("SDL_Init"));
}

void System::probe_video_mode() {
	LOG_DEBUG(("video driver: %s", SDL_GetCurrentVideoDriver()));
}

void System::deinit() {
	SDL_Quit();
}
