/*
    SDL_rotozoom - rotozoom/zoom surface helpers for sdlx.
    Clean reimplementation for SDL3 (bTanks Remastered, 2026).
    Based on the API of SDL_rotozoom 0.8 by A. Schiffler (LGPL), math
    reimplemented: nearest/bilinear sampling, 32bit RGBA and 8bit palette.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
*/

#include "sdlx.h"
#include "SDL_rotozoom.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef SMOOTHING_OFF
#define SMOOTHING_OFF		0
#endif
#ifndef SMOOTHING_ON
#define SMOOTHING_ON		1
#endif

static Uint32 rz_colorkey(const SDL_Surface *s) {
	Uint32 key = 0;
	SDL_GetSurfaceColorKey((SDL_Surface *)s, &key);
	return key;
}

/*bilinear or nearest sample of 32bit RGBA surface*/
static void rz_sample_rgba(const Uint8 *base, const int w, const int h, const int pitch,
	const float fx, const float fy, Uint8 out[4], const int smooth)
{
	int x = (int)fx, y = (int)fy;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x > w - 1) x = w - 1;
	if (y > h - 1) y = h - 1;

	const Uint8 *p = base + y * pitch + x * 4;
	if (!smooth) {
		out[0] = p[0]; out[1] = p[1]; out[2] = p[2]; out[3] = p[3];
		return;
	}

	float dx = fx - x, dy = fy - y;
	//clamp fractions at the image border so sampling stays inside
	float x1 = (x + 1 <= w - 1)? dx : 0.0f;
	float y1 = (y + 1 <= h - 1)? dy : 0.0f;

	const Uint8 *p00 = p;
	const Uint8 *p10 = (x + 1 <= w - 1)? p + 4 : p;
	const Uint8 *p01 = (y + 1 <= h - 1)? p + pitch : p;
	const Uint8 *p11 = ((x + 1 <= w - 1) && (y + 1 <= h - 1))? p + pitch + 4 : p;

	for (int c = 0; c < 4; ++c) {
		float v = p00[c] * (1 - x1) * (1 - y1) + p10[c] * x1 * (1 - y1) +
			p01[c] * (1 - x1) * y1 + p11[c] * x1 * y1;
		out[c] = (Uint8)(v + 0.5f);
	}
}

static void rz_transform_rgba(const SDL_Surface *src, SDL_Surface *dst,
	const double angle, const double zoom, const int smooth)
{
	const int sw = src->w, sh = src->h;
	const int dw = dst->w, dh = dst->h;
	const Uint8 *sp = (const Uint8 *)src->pixels;
	const int spitch = src->pitch;
	Uint8 *dp = (Uint8 *)dst->pixels;
	const int dpitch = dst->pitch;

	const double rad = angle * (M_PI / 180.0);
	const double sina = sin(rad), cosa = cos(rad);
	const double invz = (zoom != 0)? 1.0 / zoom : 1.0;
	const double cx = dw / 2.0, cy = dh / 2.0;
	const int has_key = SDL_SurfaceHasColorKey((SDL_Surface *)src);
	const Uint32 key = has_key ? rz_colorkey(src) : 0;

	for (int y = 0; y < dh; ++y) {
		Uint32 *d = (Uint32 *)(dp + y * dpitch);
		for (int x = 0; x < dw; ++x, ++d) {
			//map destination pixel back to source space
			const double ex = x + 0.5 - cx, ey = y + 0.5 - cy;
			const double sx = (ex * cosa + ey * sina) * invz + sw / 2.0;
			const double sy = (-ex * sina + ey * cosa) * invz + sh / 2.0;

			if (sx < 0 || sy < 0 || sx > sw - 1 || sy > sh - 1) {
				//outside: colorkey or fully transparent
				*d = has_key ? key : 0;
				continue;
			}

			Uint8 rgba[4];
			rz_sample_rgba(sp, sw, sh, spitch, (float)sx, (float)sy, rgba, smooth);
			if (has_key) {
				Uint8 kr, kg, kb, ka;
				SDL_GetRGBA(key, SDL_GetPixelFormatDetails(src->format), SDL_GetSurfacePalette((SDL_Surface *)src), &kr, &kg, &kb, &ka);
				if (rgba[0] == kr && rgba[1] == kg && rgba[2] == kb)
					rgba[3] = 0;
			}
			*d = SDL_MapSurfaceRGBA((SDL_Surface *)dst, rgba[0], rgba[1], rgba[2], rgba[3]);
		}
	}
}

static void rz_transform_palette(const SDL_Surface *src, SDL_Surface *dst,
	const double angle, const double zoom)
{
	const int sw = src->w, sh = src->h;
	const int dw = dst->w, dh = dst->h;
	const Uint8 *sp = (const Uint8 *)src->pixels;
	const int spitch = src->pitch;
	Uint8 *dp = (Uint8 *)dst->pixels;
	const int dpitch = dst->pitch;

	const double rad = angle * (M_PI / 180.0);
	const double sina = sin(rad), cosa = cos(rad);
	const double invz = (zoom != 0)? 1.0 / zoom : 1.0;
	const double cx = dw / 2.0, cy = dh / 2.0;
	const Uint32 key = rz_colorkey(src);

	for (int y = 0; y < dh; ++y) {
		Uint8 *d = dp + y * dpitch;
		for (int x = 0; x < dw; ++x, ++d) {
			const double ex = x + 0.5 - cx, ey = y + 0.5 - cy;
			const int sx = (int)((ex * cosa + ey * sina) * invz + sw / 2.0);
			const int sy = (int)((-ex * sina + ey * cosa) * invz + sh / 2.0);
			if (sx < 0 || sy < 0 || sx >= sw || sy >= sh)
				*d = (Uint8)key;
			else
				*d = sp[sy * spitch + sx];
		}
	}
}

void rotozoomSurfaceSize(int width, int height, double angle, double zoom, int *dstwidth, int *dstheight)
{
	double dw, dh;
	dw = fabs(zoom * (double)width * cos(angle * (M_PI / 180.0))) +
		fabs(zoom * (double)height * sin(angle * (M_PI / 180.0)));
	dh = fabs(zoom * (double)width * sin(angle * (M_PI / 180.0))) +
		fabs(zoom * (double)height * cos(angle * (M_PI / 180.0)));

	if (dw < 1) dw = 1;
	if (dh < 1) dh = 1;
	*dstwidth = (int)ceil(dw);
	*dstheight = (int)ceil(dh);
}

void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight)
{
	double dw = fabs(zoomx * width), dh = fabs(zoomy * height);
	if (dw < 1) dw = 1;
	if (dh < 1) dh = 1;
	*dstwidth = (int)ceil(dw);
	*dstheight = (int)ceil(dh);
}

static SDL_Surface *rz_convert_rgba(SDL_Surface *src, int *converted)
{
	*converted = 0;
	if (SDL_BITSPERPIXEL(src->format) == 32)
		return src;
	SDL_Surface *r = SDL_ConvertSurface(src, SDL_PIXELFORMAT_ARGB8888);
	*converted = (r != NULL);
	return r;
}

SDL_Surface *rotozoomSurface(SDL_Surface *src, double angle, double zoom, int smooth)
{
	if (src == NULL || zoom == 0)
		return NULL;

	int converted = 0;
	SDL_Surface *rgba = rz_convert_rgba(src, &converted);
	if (rgba == NULL)
		return NULL;

	if (angle > 360.0) angle -= 360.0;
	if (angle < 0.0) angle += 360.0;

	int dstw = 0, dsth = 0;
	rotozoomSurfaceSize(rgba->w, rgba->h, angle, zoom, &dstw, &dsth);

	SDL_Surface *dst = SDL_CreateSurface(dstw, dsth, SDL_PIXELFORMAT_ARGB8888);
	if (dst == NULL) {
		if (converted) SDL_DestroySurface(rgba);
		return NULL;
	}

	//copy colorkey so the outside area is transparent for keyed sources
	if (SDL_SurfaceHasColorKey(src)) {
		SDL_SetSurfaceColorKey(dst, true, rz_colorkey(src));
	}

	if (!SDL_LockSurface(rgba) || !SDL_LockSurface(dst)) {
		if (converted) SDL_DestroySurface(rgba);
		SDL_DestroySurface(dst);
		return NULL;
	}

	rz_transform_rgba(rgba, dst, angle, zoom, smooth == SMOOTHING_ON);

	SDL_UnlockSurface(dst);
	SDL_UnlockSurface(rgba);

	if (converted)
		SDL_DestroySurface(rgba);
	return dst;
}

SDL_Surface *zoomSurface(SDL_Surface *src, double zoomx, double zoomy, int smooth)
{
	if (src == NULL || zoomx == 0.0 || zoomy == 0.0)
		return NULL;

	int dstw = 0, dsth = 0;
	zoomSurfaceSize(src->w, src->h, zoomx, zoomy, &dstw, &dsth);

	const bool palette8 = (SDL_BITSPERPIXEL(src->format) == 8);
	SDL_PixelFormat fmt = palette8 ? src->format : SDL_PIXELFORMAT_ARGB8888;

	SDL_Surface *dst = SDL_CreateSurface(dstw, dsth, fmt);
	if (dst == NULL)
		return NULL;

	if (palette8) {
		//clone palette and colorkey for 8bit surfaces
		SDL_Palette *spal = SDL_GetSurfacePalette((SDL_Surface *)src);
		if (spal != NULL) {
			SDL_Palette *dpal = SDL_CreatePalette(spal->ncolors);
			if (dpal != NULL) {
				int n = spal->ncolors;
				if (n > 256) n = 256;
				SDL_SetPaletteColors(dpal, spal->colors, 0, n);
				SDL_SetSurfacePalette(dst, dpal);
			}
		}
	}
	if (SDL_SurfaceHasColorKey(src))
		SDL_SetSurfaceColorKey(dst, true, rz_colorkey(src));

	if (!SDL_LockSurface(src) || !SDL_LockSurface(dst)) {
		SDL_DestroySurface(dst);
		return NULL;
	}

	if (palette8) {
		rz_transform_palette(src, dst, 0.0, (zoomx + zoomy) / 2.0);
	} else {
		//uniform zoom through the rotate path with angle 0 keeps one code path
		int converted = 0;
		SDL_Surface *rgba = rz_convert_rgba((SDL_Surface *)src, &converted);
		if (rgba == NULL) {
			SDL_UnlockSurface(dst);
			SDL_UnlockSurface((SDL_Surface *)src);
			SDL_DestroySurface(dst);
			return NULL;
		}
		if (SDL_LockSurface(rgba)) {
			rz_transform_rgba(rgba, dst, 0.0, (zoomx + zoomy) / 2.0, smooth == SMOOTHING_ON);
			SDL_UnlockSurface(rgba);
		}
		if (converted)
			SDL_DestroySurface(rgba);
	}

	SDL_UnlockSurface(dst);
	SDL_UnlockSurface((SDL_Surface *)src);
	return dst;
}
