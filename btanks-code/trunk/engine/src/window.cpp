/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#ifdef _WINDOWS
#	include <windows.h>
#endif


#include "window.h"
#include "sdlx/keysym.h"
#include "config.h"
#include "sdlx/system.h"
#include "sdlx/sdl_ex.h"
#include "version.h"
#include "finder.h"
#include <stdlib.h>
#include <assert.h>

#ifdef _WINDOWS
#	define putenv _putenv

static STICKYKEYS g_StartupStickyKeys = {sizeof(STICKYKEYS), 0};
static TOGGLEKEYS g_StartupToggleKeys = {sizeof(TOGGLEKEYS), 0};
static FILTERKEYS g_StartupFilterKeys = {sizeof(FILTERKEYS), 0};    

void AllowAccessibilityShortcutKeys( bool bAllowKeys )
{
    if( bAllowKeys )
    {
        // Restore StickyKeys/etc to original state and enable Windows key      
        STICKYKEYS sk = g_StartupStickyKeys;
        TOGGLEKEYS tk = g_StartupToggleKeys;
        FILTERKEYS fk = g_StartupFilterKeys;
        
        SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &g_StartupStickyKeys, 0);
        SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &g_StartupToggleKeys, 0);
        SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &g_StartupFilterKeys, 0);
    }
    else
    {
        // Disable StickyKeys/etc shortcuts but if the accessibility feature is on, 
        // then leave the settings alone as its probably being usefully used
 
        STICKYKEYS skOff = g_StartupStickyKeys;
        if( (skOff.dwFlags & SKF_STICKYKEYSON) == 0 )
        {
            // Disable the hotkey and the confirmation
            skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
            skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;
 
            SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skOff, 0);
        }
 
        TOGGLEKEYS tkOff = g_StartupToggleKeys;
        if( (tkOff.dwFlags & TKF_TOGGLEKEYSON) == 0 )
        {
            // Disable the hotkey and the confirmation
            tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
            tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;
 
            SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tkOff, 0);
        }
 
        FILTERKEYS fkOff = g_StartupFilterKeys;
        if( (fkOff.dwFlags & FKF_FILTERKEYSON) == 0 )
        {
            // Disable the hotkey and the confirmation
            fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
            fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;
 
            SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fkOff, 0);
        }
    }
}
#else

#include <SDL3/SDL_opengl.h>

#endif


IMPLEMENT_SINGLETON(Window, IWindow);

IWindow::IWindow() : _fr(10.0f) {}

void IWindow::initSDL() {
	LOG_DEBUG(("initializing SDL..."));
	Uint32 subsystems = SDL_INIT_VIDEO | (_init_joystick? SDL_INIT_JOYSTICK: 0); //SDL3: timers are always initialized
	sdlx::System::init(subsystems);

	LOG_DEBUG(("SDL %d.%d.%d, video driver: %s",
		SDL_VERSIONNUM_MAJOR(SDL_GetVersion()), SDL_VERSIONNUM_MINOR(SDL_GetVersion()), SDL_VERSIONNUM_MICRO(SDL_GetVersion()),
		SDL_GetCurrentVideoDriver()));
	LOG_DEBUG(("enabling text input..."));
	//SDL3: unicode arrives through SDL_EVENT_TEXT_INPUT, autorepeat is always on

	sdlx::Surface::set_default_flags(sdlx::Surface::Hardware | sdlx::Surface::Alpha);
}

void IWindow::init(const int argc, char *argv[]) {
	_init_joystick = true;

	_fullscreen = false;
	_vsync = true;
	_vsync_active = false;
	_fsaa = 0;

	_opengl = true;
	_force_soft = false;

	bool force_gl = false;
	_explicit_size = false;
	Config->get("engine.window.width", _w, 800);
	Config->get("engine.window.height", _h, 600);
	Config->get("engine.window.fullscreen", _fullscreen, false);
	Config->get("engine.window.vsync", _vsync, true);

	for(int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--fs") == 0) _fullscreen = true;
		else if (strcmp(argv[i], "--vsync") == 0) _vsync = true;
		//else if (strcmp(argv[i], "--320x200") == 0) { _w = 320; _h = 200; }
		//else if (strcmp(argv[i], "--320x240") == 0) { _w = 320; _h = 240; }
		else if (strcmp(argv[i], "-0") == 0) { _w = 640; _h = 480; _explicit_size = true; }
		else if (strcmp(argv[i], "-1") == 0) { _w = 800; _h = 600; _explicit_size = true; }
		else if (strcmp(argv[i], "-2") == 0) { _w = 1024; _h = 768; _explicit_size = true; }
		else if (strcmp(argv[i], "-3") == 0) { _w = 1152; _h = 864; _explicit_size = true; }
		else if (strcmp(argv[i], "-4") == 0) { _w = 1280; _h = 1024; _explicit_size = true; }
		else if (strcmp(argv[i], "-5") == 0) { _w = 1920; _h = 1080; _explicit_size = true; }
		else if (strcmp(argv[i], "--fsaa") == 0) { _fsaa = (_fsaa)?(_fsaa<< 1) : 1; }
		else if (strcmp(argv[i], "--no-joystick") == 0) { _init_joystick = false; }
		else if (strcmp(argv[i], "--help") == 0) {
			printf(
					"\t-2 -3 -4 -5\t\tenlarge video mode to 1024x768, 1152x864, 1280x1024 or 1920x1080\n"
					"\t--fs\t\t\tfullscreen\n"
					"\t--vsync\t\t\tvertical synchronization\n"
				  );
			return;
		}
	}

	initSDL();

	LOG_DEBUG(("setting caption..."));
	sdlx::Surface::set_window_title("Battle tanks - " + getVersion());

	createMainWindow();

	sdlx::System::probe_video_mode();

	if (sdlx::Surface::get_window() != NULL) {
		SDL_StartTextInput(sdlx::Surface::get_window());
	}
	_running = true;
}

static int temp_nod(int a, int b) {
	int p = a % b;
	while (p != 0) {
		a = b;
		b = p;
		p = a % b;
	}
	return b;
}

void IWindow::createMainWindow() {
	int flags = sdlx::Surface::Fullscreen;
	if (!_fullscreen) flags = 0;

	TRY {
		SDL_DisplayID display = SDL_GetPrimaryDisplay();
		int count = 0;
		SDL_DisplayMode **modes = SDL_GetFullscreenDisplayModes(display, &count);
		if (modes != NULL) {
			LOG_DEBUG(("available modes:"));
			for(int i = 0; i < count; ++i) {
				int w = modes[i]->w, h = modes[i]->h;
				if (w < 800 || h < 600)
					continue;
				int nod = temp_nod(w, h);
				int aw = w / nod, ah = h / nod;
				if (w > 800 && w < 1024 && aw == 4 && ah == 3)
					continue; //skip numerous stupid modes. disable this if you want to
				LOG_DEBUG(("\t%dx%d, %d:%d", w, h, aw, ah));
				resolutions.push_front(sdlx::Rect(0, 0, w, h));
			}
			SDL_free(modes);
		}
	} CATCH("screen modes probe", {});

	//in fullscreen keep the game's virtual resolution at the classic 800x600
	//(big UI, crisp GPU scaling) unless a resolution switch was given
	if (_fullscreen && !_explicit_size) {
		_w = 800;
		_h = 600;
	}

	TRY {
		_window.set_video_mode(_w, _h, 0, flags);
	} CATCH("setting video mode", {
		LOG_WARN(("could not set up video mode, falling back to 800x600"));
		_w = 800; _h = 600;
		_window.set_video_mode(_w, _h, 0, 0);
	});

	if (_fullscreen && sdlx::Surface::get_window() != NULL) {
		//desktop resolution: the GPU letterboxes the virtual size to fit
		SDL_SetWindowFullscreenMode(sdlx::Surface::get_window(), NULL);
	}

	//the renderer exists only after set_video_mode()
	if (_vsync && sdlx::Surface::get_renderer() != NULL) {
		SDL_SetRenderVSync(sdlx::Surface::get_renderer(), 1);
		int v = 0;
		_vsync_active = SDL_GetRenderVSync(sdlx::Surface::get_renderer(), &v) && v > 0;
		LOG_DEBUG(("vsync: %s", _vsync_active? "on": "unavailable"));
	}

	sdlx::Surface::set_window_title("Battle tanks - " + getVersion());
	LOG_DEBUG(("created main surface. (%dx%dx%d, renderer: %s)", _w, _h, _window.getBPP(), SDL_GetRendererName(sdlx::Surface::get_renderer())));
}

void IWindow::run() {

	GET_CONFIG_VALUE("engine.fps-limit", int, fps_limit, 100);
	GET_CONFIG_VALUE("engine.simulation-rate", int, sim_rate, 120);
	if (sim_rate < 30) sim_rate = 30;
	if (sim_rate > 480) sim_rate = 480;

	_fr = fps_limit;
	int max_delay = 1000000 / fps_limit;
	const double sim_step = 1.0 / sim_rate;
	LOG_DEBUG(("fps_limit set to %d, maximum frame delay: %d, simulation: %d steps/s%s",
		fps_limit, max_delay, sim_rate, _vsync_active? ", vsync": ""));

	//fixed timestep with an accumulator: the simulation result no longer depends
	//on the frame rate (vsync, 60/120Hz displays, slow frames)
	Uint64 last_ns = SDL_GetTicksNS();
	double accumulator = 0;

	SDL_Event events[32];
	while (_running) {
		_timer.reset();

		//drain the event queue, then pair key downs with their text input
		//events so the engine's SDL1.2-style keysym.unicode keeps working
		int n = 0;
		while (n < (int)(sizeof(events)/sizeof(events[0])) && SDL_PollEvent(&events[n])) {
			//mouse events arrive in window coordinates: map them into the
			//renderer's logical (game) coordinate space
			switch(events[n].type) {
			case SDL_EVENT_MOUSE_MOTION:
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
			case SDL_EVENT_MOUSE_WHEEL:
				SDL_ConvertEventToRenderCoordinates(sdlx::Surface::get_renderer(), &events[n]);
			break;
			default: break;
			}
			++n;
		}

		for (int i = 0; i < n; ++i) {
			SDL_Event &event = events[i];

			event_signal.emit(event);

			switch(event.type) {
			case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
			case SDL_EVENT_JOYSTICK_BUTTON_UP:
				joy_button_signal.emit(event.jbutton.which, event.jbutton.button, event.type == SDL_EVENT_JOYSTICK_BUTTON_DOWN);
			break;

			case SDL_EVENT_KEY_UP:
			case SDL_EVENT_KEY_DOWN: {
				SDL_keysym keysym;
				keysym.scancode = (Uint8)event.key.scancode;
				keysym.sym = event.key.key;
				keysym.mod = event.key.mod;
				if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
					//attach the text from a directly-following text input event
					for (int j = i + 1; j < n; ++j) {
						if (events[j].type == SDL_EVENT_TEXT_INPUT) {
							const char *t = events[j].text.text;
							if (t != NULL && t[0] != '\0') {
								unsigned cp = (unsigned char)t[0];
								if ((cp & 0xe0) == 0xc0 && t[1]) cp = ((cp & 0x1f) << 6) | (t[1] & 0x3f);
								else if ((cp & 0xf0) == 0xe0 && t[1] && t[2]) cp = ((cp & 0x0f) << 12) | ((t[1] & 0x3f) << 6) | (t[2] & 0x3f);
								else if ((cp & 0xf8) == 0xf0 && t[1] && t[2] && t[3]) cp = ((cp & 0x07) << 18) | ((t[1] & 0x3f) << 12) | ((t[2] & 0x3f) << 6) | (t[3] & 0x3f);
								keysym.unicode = cp;
							}
							break;
						}
						if (events[j].type == SDL_EVENT_KEY_DOWN || events[j].type == SDL_EVENT_KEY_UP)
							break;
					}
				}
				key_signal.emit(keysym, event.type == SDL_EVENT_KEY_DOWN);
			}
			break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
				mouse_signal.emit(event.button.button, event.type == SDL_EVENT_MOUSE_BUTTON_DOWN, (int)event.button.x, (int)event.button.y);
				break;

			case SDL_EVENT_MOUSE_MOTION:
				mouse_motion_signal.emit(event.motion.state, (int)event.motion.x, (int)event.motion.y, (int)event.motion.xrel, (int)event.motion.yrel);
				break;

			case SDL_EVENT_MOUSE_WHEEL: {
				//translate wheel into SDL1.2-style synthetic buttons 4/5
				const int button = (event.wheel.y > 0)? SDL_BUTTON_WHEELUP : SDL_BUTTON_WHEELDOWN;
				mouse_signal.emit(button, true, (int)event.wheel.mouse_x, (int)event.wheel.mouse_y);
				mouse_signal.emit(button, false, (int)event.wheel.mouse_x, (int)event.wheel.mouse_y);
			}
			break;

			case SDL_EVENT_QUIT:
				_running = false;
			break;
			}
		}

		const Uint64 now_ns = SDL_GetTicksNS();
		double elapsed = (now_ns - last_ns) / 1e9;
		last_ns = now_ns;
		if (elapsed > 0.1) elapsed = 0.1; //window drag, breakpoint, loading hitch: do not spiral
		accumulator += elapsed;

		int steps = 0;
		while (accumulator >= sim_step && steps < 8) {
			sim_signal.emit((float)sim_step);
			accumulator -= sim_step;
			++steps;
		}
		if (accumulator >= sim_step)
			accumulator = 0; //could not catch up: drop the debt

		tick_signal.emit((float)(elapsed > 0.0005? elapsed: 0.0005));


		IWindow::flip();

		//temporary debug: dump raw framebuffer contents of given frames
		{
			static int frame_no = 0;
			const char *dump_frames = getenv("BT_DUMP_FRAMES");
			if (dump_frames != NULL) {
				if (strstr(dump_frames, (std::string(",").append(mrt::format_string("%d", frame_no)).append(",")).c_str()) != NULL ||
					strcmp(dump_frames, mrt::format_string("%d", frame_no).c_str()) == 0) {
					char fname[64];
					snprintf(fname, sizeof(fname), "/tmp/bt_frame_%06d.bmp", frame_no);
					_window.save_bmp(fname);
					LOG_DEBUG(("dumped frame %d to %s", frame_no, fname));
				}
				++frame_no;
			}
		}

		int t_delta = _timer.microdelta();

#ifdef DEBUG
		assert(t_delta >= 0);
#else
		if (t_delta < 0)
			t_delta = 0;
#endif
		if (!_vsync_active && t_delta < max_delay) {
			sdlx::Timer::microsleep("fps limit", max_delay - t_delta);
		}

		t_delta = _timer.microdelta();
		_fr = (t_delta != 0)? (1000000.0 / t_delta): 1000000;
	}
	LOG_DEBUG(("exiting main loop."));
}

void IWindow::init_dummy() {
    LOG_DEBUG(("initializing dummy video driver..."));
	setenv("SDL_VIDEODRIVER", "dummy", 1);
    sdlx::System::init(SDL_INIT_VIDEO);
	sdlx::Surface::set_default_flags(sdlx::Surface::Alpha);
    _window.set_video_mode(640, 480, 0, 0);
}

void IWindow::deinit() {
#ifdef _WINDOWS
	AllowAccessibilityShortcutKeys(true);
#endif
	_running = false;
	LOG_DEBUG(("shutting down, freeing surface"));
	_window.free();
}

IWindow::~IWindow() {
	//_window.free();
}

void IWindow::flip() {
	_window.flip();
}

void IWindow::resetTimer() {
	_timer.reset();
}
