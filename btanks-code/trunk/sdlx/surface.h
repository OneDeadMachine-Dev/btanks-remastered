#ifndef __SDL_CXX_LAYER_SURFACE_H__
#define __SDL_CXX_LAYER_SURFACE_H__

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
#include "rect.h"
#include <string>

namespace mrt {
class Chunk;
}

namespace sdlx {
	//SDL3 has no surface flags: SDL_SWSURFACE/HWSURFACE are meaningless,
	//alpha/colorkey are surface properties now. we keep our own bit values
	//for the engine's flag-based API.
	enum {
		Default = 0x7fffffff,
		Software = 0,
		Hardware = 0,
		Fullscreen = 0x00100000,
		ColorKey = 0x00010000,
		Alpha    = 0x00020000
	};

	class SDLXAPI Surface {
	public:
		//kept as class members for source compatibility (see namespace-level enum above)
		static const Uint32 Default = 0x7fffffff;
		static const Uint32 Software = 0;
		static const Uint32 Hardware = 0;
		static const Uint32 Fullscreen = 0x00100000;
		static const Uint32 ColorKey = 0x00010000;
		static const Uint32 Alpha = 0x00020000;

		static void set_default_flags(const Uint32 flags);
		static void set_window_title(const std::string &title);
		static SDL_Window * get_window();
		static SDL_Renderer * get_renderer();

		enum {
			DefaultEnum = 0x7fffffff
		};

		Surface();
		Surface(SDL_Surface *x);
		void assign(SDL_Surface *x);

		inline const bool isNull() const { return surface == NULL; }
		inline SDL_Surface * get_sdl_surface() { return surface; }
		inline SDL_Surface * get_sdl_surface() const { return surface; }

		void put_pixel(int x, int y, Uint32 pix);
		Uint32 get_pixel(int x, int y) const;

		inline void *get_pixels() const {return surface->pixels;}

		//dimensions of the virtual screen come from the logical render size
		const int get_width() const;
		const int get_height() const;
		inline const int getPitch() const { return surface->pitch; }
		inline const int getBPP() const { return SDL_BITSPERPIXEL(surface->format); }
		inline const Uint32 getFlags() const { return 0; }

		const Rect get_size() const;

		void create_rgb(int width, int height, int depth, Uint32 flags = Default);
		void create_rgb_from(void *pixels, int width, int height, int depth, int pitch = -1);
		void convert(Surface &dest, SDL_PixelFormat fmt, Uint32 flags = Default) const;
		void convert(Uint32 flags = Default);

		void get_video();
		void set_video_mode(int w, int h, int bpp, int flags = Default);

		void load_bmp(const std::string &fname);
		void save_bmp(const std::string &fname) const;
		//PNG and JPEG (stb_image)
		void load_image(const std::string &fname);
		void load_image(const mrt::Chunk &memory);
		/*
		 Quote from manual :
		 "Only the position is used in the dstrect (the width and height are
		 ignored)."
		 */
		void blit(const Surface &s, const int x, const int y);
		void blit(const Surface &s, const Rect &from, const int x, const int y);
		void blit(const Surface &s, const Rect &from); //to pos 0:0

		void update();
		void update(const Rect &rect);
		void update(const int x, const int y, const int w, const int h);
		void flip();
		void toggle_fullscreen();

		inline const Uint32 map_rgb(const Uint8 r, const Uint8 g, const Uint8 b) const {
		    return SDL_MapSurfaceRGB(surface, r, g, b);
		}

		inline const Uint32 map_rgba(const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a) const {
    		return SDL_MapSurfaceRGBA(surface, r, g, b, a);
		}

		inline void get_rgb(const Uint32 color, Uint8 &r, Uint8 &g, Uint8 &b) const {
			SDL_GetRGB(color, SDL_GetPixelFormatDetails(surface->format), SDL_GetSurfacePalette(surface), &r, &g, &b);
		}

		inline void get_rgba(const Uint32 color, Uint8 &r, Uint8 &g, Uint8 &b, Uint8 &a) const {
			SDL_GetRGBA(color, SDL_GetPixelFormatDetails(surface->format), SDL_GetSurfacePalette(surface), &r, &g, &b, &a);
		}

		void fill(Uint32 color);
		void fill_rect(const Rect &r, Uint32 color);

		bool get_color_key(Uint32 &key) const;
		void set_color_key(Uint32 key, Uint32 flag = ColorKey);
		void set_alpha(Uint8 alpha, Uint32 flags = Alpha);
		//SDL1.2 SDL_SRCALPHA on/off for surfaces with a per-pixel alpha channel:
		//blend=false makes blits copy pixels (alpha included), blend=true restores
		//normal alpha blending. does not touch the modulation value.
		void set_blend(bool blend);
		//pure modulation (fade): multiplies the per-pixel alpha, blend mode is kept
		void set_alpha_mod(Uint8 alpha);
		void display_format_alpha();
		void display_format();

		SDL_PixelFormat get_pixel_format() const { return surface->format; }

		void free();

		//invalidate the cached GPU texture after direct pixel changes
		void mark_dirty() const;

		void lock() const;
		void unlock() const;

		void set_clip_rect(const sdlx::Rect &rect);
		void reset_clip_rect();
		void get_clip_rect(sdlx::Rect &rect);

		~Surface();

		//win32 specific
		void load_from_resource(const char *name);

		//sdl_gfx

		void rotozoom(const sdlx::Surface &src, double angle, double zoom, bool smooth = true);
		void zoom(double xfactor, double yfactor, bool smooth = true);

	private:
		Surface(const Surface &x);
		const Surface& operator=(const Surface &x);
		SDL_Surface *surface;
		Uint8 _alpha;
		SDL_BlendMode _blend;
		static int default_flags;

		//resolves the GPU texture (small surfaces live in shared atlases) and the
		//source rect of the whole surface inside it
		static SDL_Texture *get_texture(const Surface &src, SDL_FRect &rect);
		static void apply_texture_state(SDL_Texture *t, const Surface &src);
		static void release_texture(SDL_Surface *key);
	};
}

#endif
