/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include "c_map.h"
#include "surface.h"

#include "mrt/logger.h"
#include "mrt/file.h"

#include "math/binary.h"
#include "math/matrix.h"

#if defined(__GNUC__)
#define restrict __restrict__
#elif !defined(restrict)
#	define restrict
#endif

using namespace sdlx;

CollisionMap::CollisionMap() : _empty(true), _full(false), _w(0), _h(0), _width(0), _data() {}


static inline const bool bitline_collide(
 const unsigned char *base1, const int size1, const int x1, 
 const unsigned char *base2, const int size2, const int x2, 
	int line_size) {
	const int shift1 = x1 % 8, shift2 = x2 % 8;
	int pos1 = x1 / 8, pos2 = x2 / 8;
	// Read bytes rather than unaligned words. A shifted final byte may need
	// its neighbour, but that neighbour must belong to the same mask row.
	for (; line_size > 0; line_size -= 8, ++pos1, ++pos2) {
		unsigned a = base1[pos1] << shift1;
		unsigned b = base2[pos2] << shift2;
		if (shift1 && pos1 + 1 < size1)
			a |= base1[pos1 + 1] >> (8 - shift1);
		if (shift2 && pos2 + 1 < size2)
			b |= base2[pos2 + 1] >> (8 - shift2);
		const unsigned mask = 0xffu << (8 - math::min(line_size, 8));
		if ((a & b & mask & 0xffu) != 0)
			return true;
	}
	return false;
}

const bool CollisionMap::collides(const sdlx::Rect &src, const CollisionMap *other, const sdlx::Rect &other_src, const int bx, const int by, const bool hidden_by_other) const {
	if (_empty || other->_empty)
		return false;

	const int aw = src.w > 0 ? src.w : (int)_width;
	const int ah = src.h > 0 ? src.h : (int)_h;
	const int bw = other_src.w > 0 ? other_src.w : (int)other->_width;
	const int bh = other_src.h > 0 ? other_src.h : (int)other->_h;

	// Clip the intersection to both source masks as well as the sprite
	// rectangles. Padding bits and the next row are never visible pixels.
	int x0 = math::max(math::max(0, bx), math::max(-src.x, bx - other_src.x));
	int y0 = math::max(math::max(0, by), math::max(-src.y, by - other_src.y));
	int x1 = math::min(math::min(aw, bx + bw),
		math::min((int)_width - src.x, bx + (int)other->_width - other_src.x));
	int y1 = math::min(math::min(ah, by + bh),
		math::min((int)_h - src.y, by + (int)other->_h - other_src.y));
	if (x0 >= x1 || y0 >= y1)
		return false;
	if (_full && other->_full)
		return true;

	const unsigned char *ptr1 = (const unsigned char *)_data.get_ptr();
	const unsigned char *ptr2 = (const unsigned char *)other->_data.get_ptr();
	static const int rows[] = {0, 4, 2, 6, 3, 7, 1, 5};
	for (int sy = 0; sy < 8; ++sy) {
		for (int y = y0 + rows[sy]; y < y1; y += 8) {
			if (bitline_collide(ptr1 + (src.y + y) * _w, _w, src.x + x0,
				ptr2 + (other_src.y + y - by) * other->_w, other->_w,
				other_src.x + x0 - bx, x1 - x0))
				return true;
		}
	}
	return false;
}

static inline const bool test_pixel(const sdlx::Surface * surface, const unsigned x, const unsigned y, const CollisionMap::Type type) {
	Uint32 pixelcolor = surface->get_pixel(x, y);

	SDL_BlendMode blend = SDL_BLENDMODE_NONE;
	SDL_GetSurfaceBlendMode(surface->get_sdl_surface(), &blend);

	switch(type) {
	case CollisionMap::OnlyOpaque:
		if (blend == SDL_BLENDMODE_BLEND) {
			Uint8 r, g, b, a;
			surface->get_rgba(pixelcolor, r, g, b, a);
			return a == 255;
		}
		{
			Uint32 key = 0;
			if (surface->get_color_key(key))
				return pixelcolor != key;
		}
		return true;

	case CollisionMap::AnyVisible:
		if (blend == SDL_BLENDMODE_BLEND) {
			Uint8 r, g, b, a;
			surface->get_rgba(pixelcolor, r, g, b, a);
			return a >= 250;
		}
		{
			Uint32 key = 0;
			if (surface->get_color_key(key))
				return pixelcolor != key;
		}
		return true;
	}

	return false;
}

void CollisionMap::create(const unsigned int w, const unsigned int h, const bool bit) {
	if (w == 0 || h == 0) {
		_w = _h = _width = 0;
		_empty = true;
		_full = false;
		_data.free();
		return;
	}
	_empty = !bit;
	_full =   bit;
	_width = w;
	
	_w = (w - 1) / 8 + 1; 
	_h = h;
	
	
	_data.set_size(_w * _h);
	_data.fill(bit?~0:0);
}

bool CollisionMap::load(unsigned int w, unsigned int h, const mrt::Chunk &data) {
	if (w == 0 || h == 0)
		return false;
	unsigned size = ((w - 1) / 8 + 1) * h;
	if (size != data.get_size()) {
		LOG_WARN(("collision data size mismatch. %ux%u = %u, got %u", w, h, size, (unsigned)data.get_size()));
		return false;
	}
	
	_data = data;
	_w = (w - 1) / 8 + 1;
	_h = h;
	_width = w;

	_empty = _full = true;
	bool e_set = false, f_set = false;
	unsigned char * pdata = (unsigned char *)_data.get_ptr();
	for(unsigned y = 0; y < h; ++y) {
		unsigned x;
		for(x = 0; x < w / 8; ++x) {
			unsigned char b = pdata[y * _w + x];
			if (b != 0) {
				_empty = false;
				e_set = true;
			}
			if (b != 0xff) {
				_full = false;
				f_set = true;
			}
			if (e_set && f_set)
				return true;
		}
		unsigned w2 = w % 8;
		if (w2 != 0) {
			unsigned char mask = (unsigned char)(0xffu << (8 - w2));
			unsigned char b = pdata[y * _w + x] & mask;
			if (b != 0) {
				_empty = false;
				e_set = true;
			}
			if (b != mask) {
				_full = false;
				f_set = true;
			}
			
			if (e_set && f_set)
				return true;
		}
	}
	
	return true;
}

void CollisionMap::init(const sdlx::Surface * surface, const Type type) {
	_empty = true;
	_full = true;
	assert(surface->get_width() != 0 && surface->get_height() != 0);
	_w = (surface->get_width() - 1) / 8 + 1;
	_h = surface->get_height();
	_width = surface->get_width();
	_data.set_size(_w * _h);
	//LOG_DEBUG(("got surface %d %d -> %d %d, allocated: %u bytes", surface->get_width(), surface->get_height(), _w, _h, (unsigned)_data.get_size()));
	_data.fill(0);
	
	surface->lock();
	unsigned char * data = (unsigned char *)_data.get_ptr();
	for(int y = 0; y < surface->get_height(); ++y) {
		for(int x = 0; x < surface->get_width(); ++x) {
			unsigned int b = 7-(x&7);
			unsigned int pos = y * _w + x / 8;
			assert(pos < _data.get_size());
	
			if (test_pixel(surface, x, y, type)) {
				data[pos] |= (1 << b);
				_empty = false;
			} else _full = false;
		}
	}
	surface->unlock();	
	//LOG_DEBUG(("built collision map (size: %u): %s", (unsigned)_data.get_size(), _data.dump().c_str()));
	//if (_empty)
	//	LOG_DEBUG(("this collision map is empty"));
}

void CollisionMap::save(const std::string &fname) const {
	mrt::File f;
	f.open(fname, "wb");
	f.write_all(_data);
	f.close();
}


void CollisionMap::project(Matrix<bool> &result, const unsigned w, const unsigned h) const {
	unsigned xs = _w / w, ys = _h / h;
	if (xs * w != _w || ys * h != _h) 
		throw_ex(("cannot project collision map %dx%d (square size: %dx%d)", _w, _h, xs, ys));
	result.set_size(h, w, false);
	unsigned char *ptr = (unsigned char *)_data.get_ptr();
	unsigned int size = _data.get_size();
	for(unsigned int y = 0; y < _h; ++y) 
		for(unsigned int x = 0; x < _w; ++x) {
			assert(x + _w * y < size);
			if (ptr[x + _w * y])
				result.set(y / ys, x / xs, true);
		}
}
