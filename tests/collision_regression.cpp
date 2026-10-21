#include "sdlx/c_map.h"
#include "sdlx/rect.h"
#include "mrt/chunk.h"
#include <cstdio>
#include <random>
#include <vector>

struct Mask {
	int w, h;
	std::vector<unsigned char> bytes;
	Mask(int width, int height, std::mt19937 &random, int kind)
		: w(width), h(height), bytes(((width + 7) / 8) * height) {
		for (size_t i = 0; i < bytes.size(); ++i)
			bytes[i] = kind == 0 ? 0 : kind == 1 ? 255 : (unsigned char)random();
	}
	bool pixel(int x, int y) const {
		return x >= 0 && y >= 0 && x < w && y < h &&
			(bytes[y * ((w + 7) / 8) + x / 8] & (128 >> (x % 8)));
	}
	void load(sdlx::CollisionMap &map) const {
		mrt::Chunk data;
		data.set_data(bytes.data(), bytes.size());
		if (!map.load(w, h, data)) throw "mask failed to load";
		bool empty = true, full = true;
		for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
			empty = empty && !pixel(x, y);
			full = full && pixel(x, y);
		}
		if (map.is_empty() != empty || map.is_full() != full)
			throw "loaded mask flags disagree with visible pixels";
	}
};

static bool reference(const Mask &a, const sdlx::Rect &src, const Mask &b,
	const sdlx::Rect &other, int bx, int by) {
	const int aw = src.w > 0 ? src.w : a.w, ah = src.h > 0 ? src.h : a.h;
	const int bw = other.w > 0 ? other.w : b.w, bh = other.h > 0 ? other.h : b.h;
	for (int y = 0; y < ah; ++y) for (int x = 0; x < aw; ++x) {
		if (x >= bx && x < bx + bw && y >= by && y < by + bh &&
			a.pixel(src.x + x, src.y + y) && b.pixel(other.x + x - bx, other.y + y - by))
			return true;
	}
	return false;
}

int main() {
	try {
		std::mt19937 random(8098);
		const int widths[] = {1, 2, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65};
		for (int trial = 0; trial < 40000; ++trial) {
			Mask a(widths[random() % 14], 1 + random() % 17, random, trial % 4);
			Mask b(widths[random() % 14], 1 + random() % 17, random, (trial / 4) % 4);
			sdlx::CollisionMap ma, mb;
			a.load(ma); b.load(mb);
			sdlx::Rect src((int)(random() % 12) - 3, (int)(random() % 8) - 2,
				trial % 5 ? 1 + random() % (a.w + 10) : 0, trial % 7 ? 1 + random() % (a.h + 4) : 0);
			sdlx::Rect other((int)(random() % 12) - 3, (int)(random() % 8) - 2,
				trial % 3 ? 1 + random() % (b.w + 10) : 0, trial % 6 ? 1 + random() % (b.h + 4) : 0);
			int bx = (int)(random() % 35) - 17, by = (int)(random() % 19) - 9;
			if (ma.collides(src, &mb, other, bx, by, false) != reference(a, src, b, other, bx, by)) {
				std::fprintf(stderr, "collision mismatch at trial %d\n", trial);
				return 1;
			}
		}
		sdlx::CollisionMap zero, solid;
		zero.create(0, 1, true); solid.create(1, 1, true);
		if (!zero.is_empty() || zero.collides(sdlx::Rect(), &solid, sdlx::Rect(), 0, 0, false)) return 1;
		std::puts("PASS: 40000 mask comparisons against an independent pixel oracle, padding, clipping and empty masks");
		return 0;
	} catch (const char *message) {
		std::fprintf(stderr, "%s\n", message);
		return 1;
	}
}
