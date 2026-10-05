#ifndef BT_ENGINE_VIDEO_CONTROL_H__
#define BT_ENGINE_VIDEO_CONTROL_H__

#include <string>
#include "control.h"
#include "sdlx/surface.h"

//was an SMPEG-driven map intro video; SMPEG does not survive the SDL3 port,
//so the control shows the static map screenshot instead.
class VideoControl : public Control {
public:
	VideoControl(const std::string &base, const std::string &name);
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual void get_size(int &w, int &h) const;

private:
	std::string base, name;
	const sdlx::Surface *screenshot;
};

#endif
