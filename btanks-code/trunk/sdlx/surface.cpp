/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
 * SDL3 port: 2026, bTanks Remastered
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#include "surface.h"
#include "sdl_ex.h"
#include <SDL3/SDL_render.h>
#include <map>
#include "mrt/chunk.h"

using namespace sdlx;

int Surface::default_flags  = Default;

//screen state: the screen surface is virtual - blits/fills targeting it are
//translated into SDL_Render* calls with a per-source texture cache, so the
//frame is composed on the GPU (Metal on macOS, WebGL2 on emscripten).
static SDL_Window *_window = NULL;
static SDL_Renderer *_renderer = NULL;
static SDL_Surface *_screen_surface = NULL; //1x1 format holder, marks the screen
static int _logical_w = 0, _logical_h = 0;
static std::string _window_title = "Battle Tanks";

//texture cache keyed by the source SDL_Surface pointer
typedef std::map<SDL_Surface *, SDL_Texture *> TextureCache;
static TextureCache _textures;


SDL_Window * Surface::get_window() { return _window; }
SDL_Renderer * Surface::get_renderer() { return _renderer; }

void Surface::set_window_title(const std::string &title) {
	_window_title = title;
	if (_window != NULL)
		SDL_SetWindowTitle(_window, title.c_str());
}

static void destroy_screen() {
	for(TextureCache::iterator i = _textures.begin(); i != _textures.end(); ++i) {
		SDL_DestroyTexture(i->second);
	}
	_textures.clear();
	_screen_surface = NULL;
	_logical_w = _logical_h = 0;
	if (_renderer != NULL) {
		SDL_DestroyRenderer(_renderer);
		_renderer = NULL;
	}
	if (_window != NULL) {
		SDL_DestroyWindow(_window);
		_window = NULL;
	}
}

void Surface::set_default_flags(const Uint32 flags) {
	if (flags == Default)
		throw_ex(("set_default_flags doesnt accept 'Default' argument"));
	default_flags = flags;
}


const int Surface::get_width() const {
	return (surface == _screen_surface)? _logical_w : surface->w;
}

const int Surface::get_height() const {
	return (surface == _screen_surface)? _logical_h : surface->h;
}

const Rect Surface::get_size() const {
	return Rect(0, 0, get_width(), get_height());
}

Surface::Surface():surface(NULL), _alpha(255), _blend(SDL_BLENDMODE_NONE) {}
Surface::Surface(SDL_Surface *x) : surface(x), _alpha(255), _blend(SDL_BLENDMODE_NONE) {}

void Surface::assign(SDL_Surface *x) {
	free();
	surface = x;
}


void Surface::get_video() {
    free();
	if (_screen_surface == NULL)
		throw_ex(("get_video() called before set_video_mode"));
    surface = _screen_surface;
}

void Surface::create_rgb(int width, int height, int depth, Uint32 flags) {
	free();
	if (flags == Default) flags = default_flags;
	if (flags == Default) throw_ex(("setup default flags before using it."));

	if (depth != 32 && depth != 24 && depth != 16 && depth != 8)
		throw_ex(("unsupported depth: %d", depth));
	surface = SDL_CreateSurface(width, height, SDL_GetPixelFormatForMasks(depth, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000));
	if(surface == NULL) throw_sdl(("SDL_CreateSurface(%d, %d, %d)", width, height, depth));
	//SDL1.2 semantics: SDL_SRCALPHA on creation enables blending for blits from it
	if ((flags & Alpha) == Alpha && depth == 32) {
		SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
		_blend = SDL_BLENDMODE_BLEND;
	}
}

void Surface::create_rgb_from(void *pixels, int width, int height, int depth,  int pitch) {
	free();

	if (pitch == -1) pitch = width;
	if (depth != 32 && depth != 24 && depth != 16 && depth != 8)
		throw_ex(("unsupported depth: %d", depth));

	surface = SDL_CreateSurfaceFrom(width, height,
		SDL_GetPixelFormatForMasks(depth, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000),
		pixels, pitch * (depth / 8));
	if(surface == NULL)
		throw_sdl(("SDL_CreateSurfaceFrom"));
}

void Surface::convert(Surface &dest, SDL_PixelFormat fmt, Uint32 flags)  const {
	SDL_Surface *x = SDL_ConvertSurface(surface, fmt);
	if (x == NULL)
		throw_sdl(("SDL_ConvertSurface"));
	dest.assign(x);
}

void Surface::convert(Uint32 flags) {
	SDL_PixelFormat fmt = (_screen_surface != NULL)? _screen_surface->format : SDL_PIXELFORMAT_ARGB8888;
	SDL_Surface *x = SDL_ConvertSurface(surface, fmt);
	if (x == NULL)
		throw_sdl(("SDL_ConvertSurface"));
	assign(x);
}

//texture cache keyed by the source SDL_Surface pointer

SDL_Texture * Surface::get_texture(const Surface &src) {
	TextureCache::iterator i = _textures.find(src.surface);
	if (i != _textures.end())
		return i->second;

	SDL_Texture *t = SDL_CreateTextureFromSurface(_renderer, src.surface);
	if (t == NULL)
		throw_sdl(("SDL_CreateTextureFromSurface"));
	SDL_SetTextureScaleMode(t, SDL_SCALEMODE_NEAREST);
	_textures[src.surface] = t;
	return t;
}

void Surface::destroy_texture(SDL_Surface *key) {
	TextureCache::iterator i = _textures.find(key);
	if (i == _textures.end())
		return;
	SDL_DestroyTexture(i->second);
	_textures.erase(i);
}

void Surface::mark_dirty() const {
	destroy_texture(surface);
}

void Surface::apply_texture_state(SDL_Texture *t, const Surface &src) {
	SDL_SetTextureAlphaMod(t, src._alpha);
	SDL_SetTextureBlendMode(t, src._blend);
}

void Surface::set_video_mode(int w, int h, int bpp, int flags) {
	if (flags == Default) flags = default_flags;
	if (flags == Default) throw_ex(("setup default flags before using it."));

	destroy_screen();

	Uint32 wflags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	if ((flags & Fullscreen) == Fullscreen)
		wflags |= SDL_WINDOW_FULLSCREEN;

	_window = SDL_CreateWindow(_window_title.c_str(), w, h, wflags);
	if (_window == NULL)
		throw_sdl(("SDL_CreateWindow(%dx%d)", w, h));

	_renderer = SDL_CreateRenderer(_window, NULL);
	if (_renderer == NULL)
		_renderer = SDL_CreateRenderer(_window, "software");
	if (_renderer == NULL)
		throw_sdl(("SDL_CreateRenderer"));
	//SDL1.2 semantics: FillRect is always opaque (alpha is ignored), so the
	//per-frame background clear actually replaces the previous frame. render
	//primitives keep blend mode NONE on purpose.
	SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_NONE);

	//virtual coordinate space: the game logic always works in wxh, whatever
	//the real window size is (free HD scaling on the GPU)
	_logical_w = w;
	_logical_h = h;
	SDL_SetRenderLogicalPresentation(_renderer, w, h, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	//1x1 holder: identifies the screen surface and provides map_rgb format
	surface = SDL_CreateSurface(1, 1, SDL_PIXELFORMAT_ARGB8888);
	if (surface == NULL)
		throw_sdl(("SDL_CreateSurface(1x1)"));
	_screen_surface = surface;
}


void Surface::put_pixel(int x, int y, Uint32 pixel) {
	if (surface->pixels == NULL)
		throw_ex(("put_pixel called on unlocked surface without pixel information"));
	if (!(x >= 0 && y >= 0 && x < surface->w && y < surface->h))
		return;
	int bpp = SDL_BYTESPERPIXEL(surface->format);
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
	case 1:
		*p = pixel;
		break;

	case 2:
		*(Uint16 *)p = pixel;
		break;

	case 3:
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		p[0] = (pixel >> 16) & 0xff;
		p[1] = (pixel >> 8) & 0xff;
		p[2] = pixel & 0xff;
#else
		p[0] = pixel & 0xff;
		p[1] = (pixel >> 8) & 0xff;
		p[2] = (pixel >> 16) & 0xff;
#endif
		break;

	case 4:
		*(Uint32 *)p = pixel;
		break;

	default:
		throw_ex(("surface has unusual BytesPP value (%d)", bpp));
	}
	mark_dirty();
}

Uint32 Surface::get_pixel(int x, int y) const{
	if (surface->pixels == NULL)
		throw_ex(("get_pixel called on unlocked surface without pixel information"));
	int bpp = SDL_BYTESPERPIXEL(surface->format);
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
	case 1:
		return *p;

	case 2:
		return *(Uint16 *)p;

	case 3:
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		return p[0] << 16 | p[1] << 8 | p[2];
#else
		return p[0] | p[1] << 8 | p[2] << 16;
#endif

	case 4:
		return *(Uint32 *)p;

	default:
		throw_ex(("surface has unusual BytesPP value (%d)", bpp));
	}
}

void Surface::load_bmp(const std::string &fname) {
	free();
	surface = SDL_LoadBMP(fname.c_str());
	if (surface == NULL)
		throw_sdl(("SDL_LoadBMP"));
}

void Surface::save_bmp(const std::string &fname)  const {
	if (surface == _screen_surface && _renderer != NULL) {
		//GPU-composed screen: read the presented frame back
		SDL_Surface *shot = SDL_RenderReadPixels(_renderer, NULL);
		if (shot == NULL)
			throw_sdl(("SDL_RenderReadPixels"));
		bool r = SDL_SaveBMP_IO(shot, SDL_IOFromFile(fname.c_str(), "wb"), true);
		SDL_DestroySurface(shot);
		if (!r)
			throw_sdl(("SDL_SaveBMP_IO"));
		return;
	}
	if (!SDL_SaveBMP_IO(surface, SDL_IOFromFile(fname.c_str(), "wb"), true))
		throw_sdl(("SDL_SaveBMP_IO"));
}


#ifndef NO_SDL_IMAGE

void Surface::load_image(const std::string &str) {
    free();
    if ((surface = IMG_Load(str.c_str())) == NULL ) throw_sdl(("IMG_Load"));
}

void Surface::load_image(const mrt::Chunk &memory) {
	free();
	SDL_IOStream *op = SDL_IOFromMem(memory.get_ptr(), memory.get_size());
	if (op == NULL) throw_sdl(("SDL_IOFromMem"));
	try {
		surface = IMG_Load_IO(op, false);
		SDL_CloseIO(op);
		op = NULL;
		if (surface == NULL)
			throw_sdl(("IMG_Load_IO"));
	} CATCH("load_image", {SDL_CloseIO(op); throw;})
}

#endif


void Surface::blit(const Surface &from, const int x, const int y) {
	if (surface == _screen_surface) {
		SDL_Texture *t = get_texture(from);
		apply_texture_state(t, from);
		const SDL_FRect dst = { (float)x, (float)y, (float)from.surface->w, (float)from.surface->h };
		SDL_RenderTexture(_renderer, t, NULL, &dst);
		return;
	}

    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = from.surface->w;
    dst.h = from.surface->h;
    if (!SDL_BlitSurface(from.surface, NULL, surface, &dst))
		throw_sdl(("SDL_BlitSurface"));
	mark_dirty();
}

void Surface::blit(const Surface &from, const Rect &fromRect, const int x, const int y) {
	if (surface == _screen_surface) {
		SDL_Texture *t = get_texture(from);
		apply_texture_state(t, from);
		const SDL_FRect src = { (float)fromRect.x, (float)fromRect.y, (float)fromRect.w, (float)fromRect.h };
		const SDL_FRect dst = { (float)x, (float)y, (float)fromRect.w, (float)fromRect.h };
		SDL_RenderTexture(_renderer, t, &src, &dst);
		return;
	}

    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = 0;
    dst.h = 0;
    if (!SDL_BlitSurface(from.surface, const_cast<Rect*>(&fromRect), surface, &dst)) throw_sdl(("SDL_BlitSurface"));
	mark_dirty();
}

void Surface::blit(const Surface &from, const Rect &fromRect) {
	blit(from, fromRect, 0, 0);
}

void Surface::update(const Rect &rect) {
	//no-op: composition is presented through the renderer on flip()
}

void Surface::update() {
	//no-op
}

void Surface::update(const int x, const int y, const int w, const int h) {
	//no-op
}

void Surface::flip() {
	if (surface == _screen_surface && _renderer != NULL) {
		SDL_RenderPresent(_renderer);
	}
}

void Surface::toggle_fullscreen() {
	if (_window == NULL)
		throw_ex(("toggle_fullscreen: no window"));
	Uint32 flags = SDL_GetWindowFlags(_window);
	SDL_SetWindowFullscreen(_window, !(flags & SDL_WINDOW_FULLSCREEN));
}

void Surface::fill(Uint32 color) {
	if (surface == _screen_surface) {
		const SDL_PixelFormatDetails *details = SDL_GetPixelFormatDetails(surface->format);
		Uint8 r, g, b, a;
		SDL_GetRGBA(color, details, NULL, &r, &g, &b, &a);
		SDL_SetRenderDrawColor(_renderer, r, g, b, a);
		SDL_RenderFillRect(_renderer, NULL);
		return;
	}
    if (!SDL_FillSurfaceRect(surface, NULL, color)) throw_sdl(("SDL_FillSurfaceRect"));
	mark_dirty();
}

void Surface::fill_rect(const Rect &r, Uint32 color) {
	if (surface == _screen_surface) {
		const SDL_PixelFormatDetails *details = SDL_GetPixelFormatDetails(surface->format);
		Uint8 r8, g8, b8, a8;
		SDL_GetRGBA(color, details, NULL, &r8, &g8, &b8, &a8);
		SDL_SetRenderDrawColor(_renderer, r8, g8, b8, a8);
		const SDL_FRect fr = { (float)r.x, (float)r.y, (float)r.w, (float)r.h };
		SDL_RenderFillRect(_renderer, &fr);
		return;
	}
    if (!SDL_FillSurfaceRect(surface, (SDL_Rect *)&r , color)) throw_sdl(("SDL_FillSurfaceRect"));
	mark_dirty();
}


void Surface::set_alpha(Uint8 alpha, Uint32 flags) {
	if (flags == Default) flags = default_flags;
	if (flags == Default) throw_ex(("setup default flags before using it."));

	if ((flags & Alpha) == Alpha) {
		_alpha = alpha;
		_blend = SDL_BLENDMODE_BLEND;
		if (!SDL_SetSurfaceAlphaMod(surface, alpha))
			throw_sdl(("SDL_SetSurfaceAlphaMod"));
		if (!SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND))
			throw_sdl(("SDL_SetSurfaceBlendMode"));
	} else {
		_alpha = 255;
		_blend = SDL_BLENDMODE_NONE;
		if (!SDL_SetSurfaceAlphaMod(surface, 255))
			throw_sdl(("SDL_SetSurfaceAlphaMod"));
		if (!SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE))
			throw_sdl(("SDL_SetSurfaceBlendMode"));
	}
}

void Surface::display_format_alpha() {
	convert();
	//SDL_DisplayFormatAlpha set SDL_SRCALPHA: blits from the surface alpha-blend
	if (surface != NULL && surface != _screen_surface) {
		SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
		_blend = SDL_BLENDMODE_BLEND;
	}
}

void Surface::display_format() {
	convert();
	if (surface != NULL && surface != _screen_surface) {
		SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
		_blend = SDL_BLENDMODE_NONE;
	}
}


void Surface::free() {
	if (surface == NULL) return;
	if (surface == _screen_surface)
		destroy_screen();
	mark_dirty();
	SDL_DestroySurface(surface);
	surface = NULL;
}

void Surface::lock() const {
	//the virtual screen cannot be locked
	if (surface == _screen_surface)
		throw_ex(("lock() on the screen surface is not supported (GPU-composed)"));
	if (SDL_MUSTLOCK(surface)) {
		if (!SDL_LockSurface(surface))
			throw_sdl(("SDL_LockSurface"));
	}
}

void Surface::unlock() const {
	if (surface == _screen_surface)
		return;
	if (SDL_MUSTLOCK(surface)) {
		SDL_UnlockSurface(surface);
	}
	//direct pixel access happened: invalidate the GPU texture
	const_cast<Surface *>(this)->mark_dirty();
}

void Surface::set_clip_rect(const sdlx::Rect &rect) {
	if (surface == _screen_surface) {
		const SDL_Rect r = { rect.x, rect.y, rect.w, rect.h };
		SDL_SetRenderClipRect(_renderer, &r);
		return;
	}
	SDL_SetSurfaceClipRect(surface, const_cast<sdlx::Rect*>(&rect));
}
void Surface::reset_clip_rect() {
	if (surface == _screen_surface) {
		SDL_SetRenderClipRect(_renderer, NULL);
		return;
	}
	SDL_SetSurfaceClipRect(surface, NULL);
}
void Surface::get_clip_rect(sdlx::Rect &rect) {
	if (surface == _screen_surface) {
		//SDL1.2: a fresh surface clips to the full surface. SDL3 reports a
		//disabled clip as an empty rect - return the logical size instead.
		if (!SDL_RenderClipEnabled(_renderer)) {
			rect = sdlx::Rect(0, 0, _logical_w, _logical_h);
			return;
		}
		SDL_Rect r = { 0, 0, 0, 0 };
		SDL_GetRenderClipRect(_renderer, &r);
		rect = sdlx::Rect(r.x, r.y, r.w, r.h);
		return;
	}
	SDL_GetSurfaceClipRect(surface, &rect);
}

Surface::~Surface() {
    free();
}

void Surface::load_from_resource(const char * lpResName) {
}

void Surface::set_color_key(Uint32 key, Uint32 flag) {
	if (!SDL_SetSurfaceColorKey(surface, flag != 0, key))
		throw_sdl(("SDL_SetColorKey"));
	mark_dirty();
}

bool Surface::get_color_key(Uint32 &key) const {
	return SDL_GetSurfaceColorKey(surface, &key);
}

#include "gfx/SDL_rotozoom.h"

void Surface::rotozoom(const sdlx::Surface &src, double angle, double zoom, bool smooth) {
	if (src.isNull())
		throw_ex(("rotozooming null surface"));

	free();
	int dstwidth = 0, dstheight = 0;
	::rotozoomSurfaceSize(src.get_width(), src.get_height(), angle, zoom, &dstwidth, &dstheight);
	if (dstwidth <= 0 || dstheight <= 0)
		throw_ex(("rotozoomSurfaceSize returns invalid size: %dx%d", dstwidth, dstheight));

	SDL_Surface * r = ::rotozoomSurface((SDL_Surface *)src.get_sdl_surface(), angle, zoom, smooth? SMOOTHING_ON: SMOOTHING_OFF);
	if (r == NULL)
		throw_sdl(("rotozoomSurface(%dx%d, %g, %g, %s)", src.get_width(), src.get_height(), angle, zoom, smooth?"true":"false"));
	assign(r);
}

void Surface::zoom(double xfactor, double yfactor, bool smooth) {
	if (surface == NULL)
		throw_ex(("rotozooming null surface"));
	SDL_Surface * r = zoomSurface(surface, xfactor, yfactor, smooth);
	if (r == NULL)
		throw_sdl(("zoomSurface"));
	free();
	surface = r;
}
