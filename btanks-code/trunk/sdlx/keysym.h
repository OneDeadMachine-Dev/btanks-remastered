#ifndef __SDLX_KEYSYM_COMPAT_H__
#define __SDLX_KEYSYM_COMPAT_H__

/* SDL1.2-style keysym for the engine's key_signal.
 * unicode is filled by the event loop from the SDL_EVENT_TEXT_INPUT event
 * that follows the key down (see window.cpp). */

#include <SDL3/SDL.h>

struct SDL_keysym {
	int scancode;
	SDL_Keycode sym;
	Uint16 mod;
	Uint32 unicode;

	SDL_keysym() : scancode(0), sym(SDLK_UNKNOWN), mod(0), unicode(0) {}
};

//key codes stored in old config files are SDL1.2 ones (arrows 273.., modifiers 303..):
//SDL3 uses 0x40000000+ for them. letters/digits/space/return are the same in both.
inline int sdl12_to_sdl3_keycode(const int k) {
	if (k < 256 || k >= 0x40000000)
		return k; //ASCII or already SDL3
	switch(k) {
	case 256: return SDLK_KP_0; case 257: return SDLK_KP_1; case 258: return SDLK_KP_2;
	case 259: return SDLK_KP_3; case 260: return SDLK_KP_4; case 261: return SDLK_KP_5;
	case 262: return SDLK_KP_6; case 263: return SDLK_KP_7; case 264: return SDLK_KP_8;
	case 265: return SDLK_KP_9; case 266: return SDLK_KP_PERIOD; case 267: return SDLK_KP_DIVIDE;
	case 268: return SDLK_KP_MULTIPLY; case 269: return SDLK_KP_MINUS; case 270: return SDLK_KP_PLUS;
	case 271: return SDLK_KP_ENTER; case 272: return SDLK_KP_EQUALS;
	case 273: return SDLK_UP; case 274: return SDLK_DOWN; case 275: return SDLK_RIGHT; case 276: return SDLK_LEFT;
	case 277: return SDLK_INSERT; case 278: return SDLK_HOME; case 279: return SDLK_END;
	case 280: return SDLK_PAGEUP; case 281: return SDLK_PAGEDOWN;
	case 282: return SDLK_F1; case 283: return SDLK_F2; case 284: return SDLK_F3; case 285: return SDLK_F4;
	case 286: return SDLK_F5; case 287: return SDLK_F6; case 288: return SDLK_F7; case 289: return SDLK_F8;
	case 290: return SDLK_F9; case 291: return SDLK_F10; case 292: return SDLK_F11; case 293: return SDLK_F12;
	case 300: return SDLK_NUMLOCKCLEAR; case 301: return SDLK_CAPSLOCK; case 302: return SDLK_SCROLLLOCK;
	case 303: return SDLK_RSHIFT; case 304: return SDLK_LSHIFT; case 305: return SDLK_RCTRL; case 306: return SDLK_LCTRL;
	case 307: return SDLK_RALT; case 308: return SDLK_LALT; case 309: return SDLK_RGUI; case 310: return SDLK_LGUI;
	default: return SDLK_UNKNOWN;
	}
}

//SDL1.2 wheel "buttons"; the SDL3 event loop translates SDL_EVENT_MOUSE_WHEEL
//into these so all consumers keep working
#ifndef SDL_BUTTON_WHEELUP
#define SDL_BUTTON_WHEELUP 4
#endif
#ifndef SDL_BUTTON_WHEELDOWN
#define SDL_BUTTON_WHEELDOWN 5
#endif

#endif
