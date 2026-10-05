#include "file_rw.h"
#include "mrt/base_file.h"
#include "mrt/chunk.h"
#include <SDL3/SDL_iostream.h>
#include "sdl_ex.h"
#include <assert.h>

static Sint64 mrt_size(void *context) {
	mrt::BaseFile *file = (mrt::BaseFile *)context;
	TRY {
		long pos = file->tell();
		file->seek(0, SEEK_END);
		long size = file->tell();
		file->seek(pos, SEEK_SET);
		return size;
	} CATCH("mrt_size", return -1);
}

static Sint64 mrt_seek(void *context, Sint64 offset, SDL_IOWhence whence) {
	mrt::BaseFile *file = (mrt::BaseFile *)context;
	TRY {
		file->seek((long)offset, (int)whence);
		return file->tell();
	} CATCH("mrt_seek", return -1);
}

static size_t mrt_read(void *context, void *ptr, size_t size, SDL_IOStatus *status) {
	mrt::BaseFile *file = (mrt::BaseFile *)context;
	TRY {
		int r = file->read(ptr, (int)size);
		if (r < (int)size)
			*status = SDL_IO_STATUS_EOF;
		return (size_t)(r > 0 ? r : 0);
	} CATCH("mrt_read", {
		if (status != NULL)
			*status = SDL_IO_STATUS_ERROR;
		return 0;
	});
}

static size_t mrt_write(void *context, const void *ptr, size_t size, SDL_IOStatus *status) {
	mrt::BaseFile *file = (mrt::BaseFile *)context;
	TRY {
		mrt::Chunk data;
		data.set_data((void *)ptr, size, false);
		file->write(data);
		return size;
	} CATCH("mrt_write", {
		if (status != NULL)
			*status = SDL_IO_STATUS_ERROR;
		return 0;
	});
}

static bool mrt_close(void *context) {
	mrt::BaseFile *file = (mrt::BaseFile *)context;
	TRY {
		file->close();
		return true;
	} CATCH("mrt_close", return false;);
}

SDL_IOStream * sdlx::RWFromMRTFile(mrt::BaseFile *file) {
	SDL_IOStreamInterface iface;
	SDL_INIT_INTERFACE(&iface);
	iface.size  = &mrt_size;
	iface.seek  = &mrt_seek;
	iface.read  = &mrt_read;
	iface.write = &mrt_write;
	iface.close = &mrt_close;

	SDL_IOStream * op = SDL_OpenIO(&iface, (void *)file);
	if (op == NULL)
		throw_sdl(("SDL_OpenIO()"));
	return op;
}
