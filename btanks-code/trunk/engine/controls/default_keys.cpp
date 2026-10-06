	//default keyboard layouts: {up, down, left, right, fire, alt-fire, disembark, hint-control}
#if defined(__APPLE__) || defined(__EMSCRIPTEN__)
	//Ctrl+arrows are system shortcuts on macOS (Spaces, Mission Control) and never reach the
	//game; MacBooks have no right Ctrl either, so fire is on Space and the right Option key
	static int keys[3][8] = {
		{SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_SPACE, SDLK_LSHIFT, SDLK_F1, SDLK_1},
		{SDLK_R, SDLK_F, SDLK_D, SDLK_G, SDLK_Q, SDLK_A, SDLK_F1, SDLK_1},
		{SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_RALT, SDLK_RSHIFT, SDLK_F2, SDLK_0},
	};
#else
	static int keys[3][8] = {
		{SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_LCTRL, SDLK_LSHIFT, SDLK_F1, SDLK_1},
		{SDLK_R, SDLK_F, SDLK_D, SDLK_G, SDLK_Q, SDLK_A, SDLK_F1, SDLK_1},
		{SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_RCTRL, SDLK_RSHIFT, SDLK_F2, SDLK_0},
	};
#endif
