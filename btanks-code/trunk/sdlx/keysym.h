#ifndef __SDLX_KEYSYM_COMPAT_H__
#define __SDLX_KEYSYM_COMPAT_H__

/* SDL1.2-style keysym for the engine's key_signal.
 * unicode is filled by the event loop from the SDL_EVENT_TEXT_INPUT event
 * that follows the key down (see window.cpp). */

#include <SDL3/SDL.h>

struct SDL_keysym {
	Uint8 scancode;
	SDL_Keycode sym;
	Uint16 mod;
	Uint32 unicode;

	SDL_keysym() : scancode(0), sym(SDLK_UNKNOWN), mod(0), unicode(0) {}
};

//SDL1.2 wheel "buttons"; the SDL3 event loop translates SDL_EVENT_MOUSE_WHEEL
//into these so all consumers keep working
#ifndef SDL_BUTTON_WHEELUP
#define SDL_BUTTON_WHEELUP 4
#endif
#ifndef SDL_BUTTON_WHEELDOWN
#define SDL_BUTTON_WHEELDOWN 5
#endif

#endif
