# Battle Tanks Remastered: нативный рендер (Metal) + WebGL-порт

## Архитектурное решение

Переносим sdlx с SDL 1.2 на **SDL3**, рендер — **SDL_Renderer**: на macOS он использует **нативный Metal** (GPU-батчинг), в браузере через Emscripten — **WebGL2**. Один рендер-код для обеих целей. glSDL (fixed-function OpenGL 2010 года) и d3dsdl удаляются — они тупиковые ветки.

Почему это дёшево: исследования показали, что вся отрисовка движка идёт через 3 сигнатуры `sdlx::Surface::blit` + `fill`, colorkey в рендере не используется, попиксельного чтения ЭКРАНА нет (только offscreen: радар, нарезка тайлов, метрики шрифтов, коллизии — всё остаётся на CPU-поверхностях). ~101 вызов blit в 30 файлах менять не нужно вообще — меняется реализация внутри sdlx.

## Этап 1 — Миграция на SDL3 (рендер пока CPU-композиция + GPU-вывод)

1. **Базлин**: зафиксировать текущее рабочее состояние в git проекта (каталог btanks-code добавить в git).
2. **Зависимости**: brew `sdl3`; SDL3_image собрать из исходников в `~/btanks-deps` (как SDL_image); Lua 5.1/expat/zlib остаются.
3. **sdlx** (~1100 SDL-зависимых строк):
   - `surface.h/cpp`: непрозрачный SDL_Surface API (`SDL_GetSurfaceWidth`...), `SDL_ConvertSurfaceFormat` вместо `DisplayFormat(Alpha)`, `SDL_SetSurfaceAlphaMod/BlendMode` вместо `SDL_SetAlpha`, `SDL_IOStream` вместо RWops (включая file_rw.cpp), `SDL_SaveBMP_IO`
   - `window`/`system`: `SDL_CreateWindow` + `SDL_CreateRenderer` + streaming-текстура 800×600 — кадр композится на CPU (как сейчас) и заливается в GPU-текстуру → **вывод уже через Metal**; флип = `SDL_RenderPresent`
   - события (`window.cpp`): SDL3 event types, `SDL_Keycode` вместо SDLKey, `SDL_EVENT_TEXT_INPUT` вместо `keysym.unicode` (заденет ~15 файлов меню — механическая замена)
   - `joystick.cpp`: SDL3 joystick API; `thread/mutex/semaphore`: переименования (`SDL_KillThread` удалён — использовался только сетью); `cursor.cpp`: `SDL_CursorVisible/WarpMouseInWindow`
   - удалить `wrappers/glSDL*`, `wrappers/d3dsdl*`, дефайн USE_GLSDL, хаки `#pragma push_macro` в window.cpp
4. **clunk**: `SDL_OpenAudio` → `SDL_OpenAudioDeviceStream` (один файл context.cpp, ~50 строк)
5. **SMPEG**: выпилить (мёртвая библиотека, не портируется) — экран видео-настроек заменить на упрощённый; интро-катсцены пока пропускаются
6. **Сборка**: SConstruct — платформа sdl3, убрать smpeg
7. **Редактор (btanks_editor)** не переносим на этом этапе (остаётся в legacy)

✔ Веха 1: игра запускается и выглядит как сейчас, но на SDL3 напрямую, вывод кадра через Metal.

## Этап 2 — GPU-спрайтовый рендерер

Экранная поверхность sdlx становится обёрткой над SDL_Renderer: `blit` → `SDL_RenderCopy` с кэшем GPU-текстур (ключ = SDL_Surface*, сброс по явной пометке dirty: радар, fadeout, вращение), `fill` → `SDL_RenderFillRect`, `set_clip_rect` → clip-рендерера (split-screen сохраняем), `set_alpha` → `SDL_SetTextureAlphaMod`.

- fadeout объектов: попиксельная копия альфы (object.cpp:526-540) заменяется альфа-модуляцией
- RotatingObject: CPU rotozoom-кэш → `SDL_RenderCopyEx` с углом
- `SDL_RenderSetLogicalSize`: виртуальные 800×600 масштабируются в любое окно/Retina — бесплатный «HD-ремастер»
- пиксельная сетка: point-фильтрация текстур
- offscreen (радар, map-скриншот Shift+M, шрифты, тайлы) остаётся на CPU — без изменений

✔ Веха 2: кадр собирается GPU, окно ресайзится без потери чёткости, FPS выше.

## Этап 3 — Emscripten/WebGL

- Сборка emcc: Lua 5.1, expat, zlib (порты Emscripten), SDL3/SDL3_image (порты/исходники), clunk на скалярном FFT (SSE выключен)
- **bt_objects влинковывается статически** (регистрация объектов уже через статические конструкторы — механизм совместим, loadPlugins под `#ifdef`)
- сеть: `RTConfig->disable_network = true` для веб-сборки (механизм уже в движке, синглплеер/кампании работают офлайн)
- главный цикл: `IWindow::run()` разбивается на `step()`, в вебе — `emscripten_set_main_loop`
- файлы: `--preload-file data@data` + `-sFORCE_FILESYSTEM` (zip-пак resources.dat игра уже умеет читать — запасной вариант); конфиг в MEMFS (потом IDBFS)
- результат: index.html + btanks.js + data-пак, синглплеер в браузере

✔ Веха 3: играPlayable в браузере (синглплеер).

## Риски (известны заранее)

- Поведенческая чёткость событий ввода SDL1.2→SDL3 (unicode, key repeat) — проверим на диалогах меню/чата
- Семантика блендинга per-surface vs per-pixel alpha при переносе в RenderCopy — проверяем попиксельно дампами кадров (инфраструктура BT_DUMP_FRAMES уже есть)
- Мультиплеер в вебе — отдельный будущий проект (WebSocket-шлюз)

Начинаю с Этапа 1: базлин в git, зависимости, миграция sdlx.