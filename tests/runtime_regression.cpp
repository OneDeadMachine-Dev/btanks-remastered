#include <SDL3/SDL.h>
#include "mrt/file.h"
#include "mrt/serializator.h"
#include "mrt/chunk.h"
#include "net/scanner.h"
#include "sdlx/surface.h"
#include "sdlx/cursor.h"
#include "sdlx/joystick.h"
#include "controls/joyplayer.h"
#include "player_slot.h"
#include "config.h"
#include "window.h"
#include "world.h"
#include "object.h"
#include <cmath>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <csignal>
#include <cstring>
#include <sys/resource.h>

static void require(bool value, const char *message) {
	if (!value) throw std::runtime_error(message);
}
static std::string read(const std::string &path) {
	std::ifstream in(path);
	return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

class PendingObject : public Object {
public:
	static int destroyed;
	PendingObject() : Object("pending-regression") {}
	~PendingObject() { ++destroyed; }
};
int PendingObject::destroyed = 0;

int main(int argc, char **argv) {
	try {
		require(argc == 2, "pass a disposable test directory");
		const std::string path = std::string(argv[1]) + "/bt.xml";
		mrt::File::write_atomic(path, "original-config");
		struct rlimit previous, limited;
		require(getrlimit(RLIMIT_FSIZE, &previous) == 0, "getrlimit failed");
		limited = previous; limited.rlim_cur = 128;
		auto handler = signal(SIGXFSZ, SIG_IGN);
		require(setrlimit(RLIMIT_FSIZE, &limited) == 0, "setrlimit failed");
		bool failed = false;
		try { mrt::File::write_atomic(path, std::string(8192, 'x')); }
		catch (const std::exception &) { failed = true; }
		setrlimit(RLIMIT_FSIZE, &previous); signal(SIGXFSZ, handler);
		require(failed && read(path) == "original-config", "failed save changed the original config");
		for (const auto &entry : std::filesystem::directory_iterator(argv[1]))
			require(entry.path().filename() == "bt.xml", "failed write left a temporary file");
		mrt::File::write_atomic(path, "<config/>\n");
		Config->load(path);
		Config->set("engine.profile", "repair");
		Config->set("profile.repair.name", "A & B <test>");
		Config->save();
		require(read(path).find("A &amp; B &lt;test&gt;") != std::string::npos, "config XML did not survive atomic save");
		std::puts("PASS: atomic save, failed-write preservation/cleanup and config XML");

		const float values[] = {0, 1, -1, 0.125f, -0.125f, 1000000, -1000000, 1e20f, -1e20f, 1e-20f};
		for (float value : values) {
			mrt::Serializator write; mrt::Chunk bytes;
			write.add(value); write.finalize(bytes);
			mrt::Serializator input(&bytes); float result;
			input.get(result);
			require(std::fabs(result - value) <= std::fabs(value) * 0.00001f, "float roundtrip failed");
		}
		std::puts("PASS: float roundtrips including positive exponents");
		for (int prefix = 0; prefix < 4; ++prefix) {
			const int integers[] = {257, 65536, -257, -1000000, 2147483647};
			mrt::Serializator write; mrt::Chunk bytes;
			for (int i = 0; i < prefix; ++i) write.add(0);
			for (int value : integers) write.add(value);
			write.finalize(bytes); mrt::Serializator input(&bytes); int result;
			for (int i = 0; i < prefix; ++i) input.get(result);
			for (int value : integers) { input.get(result); require(result == value, "integer roundtrip failed"); }
		}
		std::puts("PASS: integer roundtrips at every byte alignment");

		{
			mrt::XMLParser::Attrs attrs, copied;
			attrs["name"] = "map & server";
			copied["stale"] = "discard";
			copied = attrs;
			attrs["name"] = "changed";
			require(copied.size() == 1 && copied["name"] == "map & server", "XML attribute assignment was not independent");
			Scanner::HostMap hosts, snapshot;
			mrt::Socket::addr address;
			hosts[address].name = "server";
			snapshot = hosts;
			hosts.clear();
			require(snapshot.size() == 1 && snapshot.begin()->second.name == "server", "server discovery snapshot was not copied");

			const std::map<std::string, int> source = {{"zeta", -257}, {"alpha", 65536}};
			mrt::Serializator write, expected;
			mrt::Chunk bytes, expected_bytes;
			write.add(source); write.finalize(bytes);
			expected.add(2u);
			expected.add(std::string("alpha")); expected.add(65536);
			expected.add(std::string("zeta")); expected.add(-257);
			expected.finalize(expected_bytes);
			require(bytes.get_size() == expected_bytes.get_size() &&
				std::memcmp(bytes.get_ptr(), expected_bytes.get_ptr(), bytes.get_size()) == 0,
				"map serialization changed the ordered count/key/value wire format");
			mrt::Serializator input(&bytes);
			std::map<std::string, int> restored = {{"stale", 1}};
			input.get(restored);
			require(restored == source, "map roundtrip failed to replace existing entries");
		}
		std::puts("PASS: XML/server map assignment and unchanged ordered map serialization");

		NetStats clocks;
		require(clocks.getAge(60000, 2000) == 0, "unsynchronized clocks used for rewind");
		clocks.updateDelta(-58000);
		require(std::fabs(clocks.getAge(60000, 1990) - 0.01f) < 0.00001f, "clock offset was not applied");
		require(std::fabs(clocks.getAge(60000, 2010) + 0.01f) < 0.00001f, "future timestamp wrapped to a large age");
		clocks.clear(); clocks.updateDelta(0);
		require(std::fabs(clocks.getAge(5, 0xfffffffbu) - 0.01f) < 0.00001f, "timer rollover was not handled");
		clocks.clear();
		require(clocks.getAge(60000, 2000) == 0, "clock reset kept old synchronization");
		std::puts("PASS: initial clock synchronization, signed age and timer rollover");

		World->push(92, new PendingObject, v2<float>());
		World->clear();
		require(PendingObject::destroyed == 1, "clearing a world leaked a pending push object");
		World->purge(0);
		require(!World->exists(92), "a command from the previous map entered the new world");
		World->clear();
		require(PendingObject::destroyed == 1, "pending object was deleted twice");
		std::puts("PASS: map clear discards pending commands and frees their objects");

		require(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK), SDL_GetError());
		{
			sdlx::Surface screen;
			screen.set_video_mode(640, 480, 0, 0);
			SDL_SetWindowSize(sdlx::Surface::get_window(), 1280, 720);
			SDL_PumpEvents();
			SDL_WarpMouseInWindow(sdlx::Surface::get_window(), 640, 360);
			SDL_PumpEvents();
			int x, y; sdlx::Cursor::get_position(x, y);
			require(std::abs(x - 320) <= 1 && std::abs(y - 240) <= 1, "cursor is not in letterboxed game coordinates");
			sdlx::Cursor::Enable(); require(sdlx::Cursor::enabled(), "show cursor failed");
			sdlx::Cursor::Disable(); require(!sdlx::Cursor::enabled(), "hide cursor failed");
			std::puts("PASS: cursor visibility and logical coordinates in a scaled letterboxed window");

			SDL_VirtualJoystickDesc desc; SDL_INIT_INTERFACE(&desc);
			desc.naxes = 2; desc.nbuttons = 2; desc.nhats = 1; desc.name = "Repair virtual";
			SDL_JoystickID id = SDL_AttachVirtualJoystick(&desc);
			require(id != 0, "virtual joystick failed");
			Config->set("profilerepair.controls.joystick.Repair virtual.fire", "b0");
			Config->set("profilerepair.controls.joystick.Repair virtual.right", "a+0");
			Config->set("profilerepair.controls.joystick.Repair virtual.up", "h0 1");
			{
				JoyPlayer player(0); PlayerSlot slot; PlayerState state;
				SDL_Event event = {};
				event.type = SDL_EVENT_JOYSTICK_BUTTON_DOWN;
				event.jbutton.which = id + 100; event.jbutton.button = 0; event.jbutton.down = true;
				Window->event_signal.emit(event); player.updateState(slot, state, 0.01f);
				require(!state.fire, "another joystick's button was accepted");
				event.jbutton.which = id;
				Window->event_signal.emit(event); player.updateState(slot, state, 0.01f);
				require(state.fire, "the opened joystick's button was ignored");
				event.type = SDL_EVENT_JOYSTICK_BUTTON_UP; event.jbutton.down = false;
				Window->event_signal.emit(event); player.updateState(slot, state, 0.01f);
				require(!state.fire, "joystick button release was ignored");
				event.type = SDL_EVENT_JOYSTICK_AXIS_MOTION;
				event.jaxis.which = id; event.jaxis.axis = 0; event.jaxis.value = 32767;
				Window->event_signal.emit(event); player.updateState(slot, state, 0.01f);
				require(state.right, "the opened joystick's axis was ignored");
				event.type = SDL_EVENT_JOYSTICK_HAT_MOTION;
				event.jhat.which = id; event.jhat.hat = 0; event.jhat.value = SDL_HAT_UP;
				Window->event_signal.emit(event); player.updateState(slot, state, 0.01f);
				require(state.up, "the opened joystick's hat was ignored");
			}
			SDL_DetachVirtualJoystick(id);
			std::printf("PASS: JoyPlayer button/axis/hat routing and release, joystick index 0, instance %u\n", id);
		}
		SDL_Quit();
		return 0;
	} catch (const std::exception &error) {
		std::fprintf(stderr, "%s\n", error.what());
		return 1;
	}
}
