# Battle Tanks Remastered — веб-сборка (Emscripten / WebGL2 через SDL3).
# Всё (движок, объекты, звук, Lua, expat) линкуется статически в один wasm.
#
#   emcmake cmake -S web -B build-web -G Ninja && cmake --build build-web
#   python3 -m http.server -d build-web/out
cmake_minimum_required(VERSION 3.24)
project(btanks_web C CXX)

if(NOT EMSCRIPTEN)
	message(FATAL_ERROR "Эта сборка только для Emscripten: используйте emcmake cmake ...")
endif()

set(TRUNK ${CMAKE_CURRENT_SOURCE_DIR}/../btanks-code/trunk)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- expat (в Emscripten нет порта) -------------------------------------------
set(EXPAT_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(EXPAT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(EXPAT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(EXPAT_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(EXPAT_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(expat
	URL https://github.com/libexpat/libexpat/releases/download/R_2_9_0/expat-2.9.0.tar.gz)
FetchContent_MakeAvailable(expat)

# --- Lua 5.1 (вшит) -----------------------------------------------------------
file(GLOB LUA_SRC ${TRUNK}/third_party/lua/*.c)
add_library(lua51 STATIC ${LUA_SRC})
target_include_directories(lua51 PUBLIC ${TRUNK}/third_party/lua)
target_compile_options(lua51 PRIVATE -w -fwasm-exceptions -sSUPPORT_LONGJMP=wasm)

# --- игра ---------------------------------------------------------------------
file(GLOB MRT_SRC ${TRUNK}/mrt/*.cpp)
file(GLOB SDLX_SRC ${TRUNK}/sdlx/*.cpp ${TRUNK}/sdlx/gfx/*.c)
list(FILTER SDLX_SRC EXCLUDE REGEX "c_test\\.cpp|ttf\\.cpp")
file(GLOB CLUNK_SRC ${TRUNK}/clunk/*.cpp ${TRUNK}/clunk/*.c)
list(FILTER CLUNK_SRC EXCLUDE REGEX "/test\\.cpp|sse_fft_context")
file(GLOB ENGINE_SRC
	${TRUNK}/engine/ai/*.cpp ${TRUNK}/engine/controls/*.cpp ${TRUNK}/engine/luaxx/*.cpp
	${TRUNK}/engine/menu/*.cpp ${TRUNK}/engine/net/*.cpp ${TRUNK}/engine/sound/*.cpp
	${TRUNK}/engine/src/*.cpp ${TRUNK}/engine/tmx/*.cpp)
list(FILTER ENGINE_SRC EXCLUDE REGEX "controls/default_keys\\.cpp")
file(GLOB OBJECTS_SRC ${TRUNK}/objects/*.cpp)

add_executable(btanks ${MRT_SRC} ${SDLX_SRC} ${CLUNK_SRC} ${ENGINE_SRC} ${OBJECTS_SRC})

target_include_directories(btanks PRIVATE
	${TRUNK} ${TRUNK}/engine ${TRUNK}/engine/src ${TRUNK}/third_party/stb
	${TRUNK}/third_party/lua ${TRUNK}/sdlx ${TRUNK}/sdlx/gfx ${TRUNK}/clunk ${TRUNK}/mrt)

# все *API-макросы (dllexport/dllimport) пустые: статическая сборка
target_compile_definitions(btanks PRIVATE
	MRTAPI= SDLXAPI= CLUNKAPI= BTANKSAPI=
	V3_DISABLE_Z _REENTRANT RELEASE ENABLE_LUA
	VERSION="0.9.8098" REVISION=8098)

# исключения движок использует повсеместно (TRY/CATCH, throw_ex)
set(WASM_FLAGS -fwasm-exceptions -sSUPPORT_LONGJMP=wasm --use-port=sdl3 --use-port=vorbis --use-port=ogg --use-port=zlib)
target_compile_options(btanks PRIVATE ${WASM_FLAGS} -O2 -Wno-deprecated -Wno-c++11-narrowing)
target_link_options(btanks PRIVATE ${WASM_FLAGS}
	-sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=268435456 -sSTACK_SIZE=1048576
	-sFORCE_FILESYSTEM=1 -sEXIT_RUNTIME=0 -lidbfs.js
	-sEXPORTED_FUNCTIONS=_main,_bt_save_config -sEXPORTED_RUNTIME_METHODS=ccall,FS,IDBFS
	--preload-file ${TRUNK}/data@/data
	--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html)
target_link_libraries(btanks PRIVATE lua51 expat)

set_target_properties(btanks PROPERTIES
	LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shell.html
	SUFFIX .html
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out
	OUTPUT_NAME index)
