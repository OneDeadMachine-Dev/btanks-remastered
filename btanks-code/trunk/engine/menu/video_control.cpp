#include "video_control.h"
#include "resource_manager.h"
#include "finder.h"
#include "sdlx/surface.h"

VideoControl::VideoControl(const std::string &base, const std::string &name) :
base(base), name(name), screenshot(NULL)
{
	std::string fname = "maps/" + name + ".jpg";
	if (Finder->exists(base, fname)) {
		screenshot = ResourceManager->load_surface("../" + fname);
	} else
		screenshot = ResourceManager->load_surface("../maps/null_video.png");
}

void VideoControl::render(sdlx::Surface &surface, const int x, const int y) const {
	surface.blit(*screenshot, x, y);
}

void VideoControl::get_size(int &w, int &h) const {
	w = screenshot->get_width();
	h = screenshot->get_height();
}
