/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
 * SDL3 port: 2026, bTanks Remastered
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */


#include "semaphore.h"
#include "sdl_ex.h"

using namespace sdlx;

Semaphore::Semaphore(const Uint32 value) : _sem (SDL_CreateSemaphore(value)) {
	if (_sem == NULL)
		throw_sdl(("SDL_CreateSemaphore"));
}

void Semaphore::post() {
	SDL_SignalSemaphore(_sem);
}

void Semaphore::wait() {
	SDL_WaitSemaphore(_sem);
}

const bool Semaphore::wait(const Uint32 timeout) {
	return SDL_WaitSemaphoreTimeout(_sem, timeout);
}

const bool Semaphore::try_wait() {
	return SDL_TryWaitSemaphore(_sem);
}

Semaphore::~Semaphore() {
	SDL_DestroySemaphore(_sem);
	_sem = NULL;
}
