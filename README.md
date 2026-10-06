# Battle Tanks Remastered

Ремастер Battle Tanks (btanks r8098, 2006–2009): нативный запуск на macOS (Apple Silicon, Metal через SDL3), затем веб-версия (WebGL2 через Emscripten).

## Сборка и запуск (macOS)

```sh
brew install sdl3 libvorbis expat scons pkgconf
./build-mac.sh      # собирает всё в btanks-code/trunk (Lua 5.1 вшит в third_party/lua)
./run-mac.sh        # запуск из дерева разработки, откуда угодно
./make-app.sh       # самодостаточный dist/"Battle Tanks.app" (52 МБ, все dylib внутри)
```

Игра сама находит данные (`data/`) и библиотеки рядом с бинарником или внутри `.app`, поэтому текущая папка не важна. Приложение подписано ad-hoc: оно запускается на этом Mac. Чтобы раздавать другим, нужны Apple Developer ID и нотаризация.

## Веб-версия

```sh
brew install emscripten cmake ninja
./make-web.sh serve     # сборка в build-web/out и сервер на http://localhost:8765/index.html?fps
```

Весь движок, объекты, звук, Lua и expat линкуются статически в один wasm (WebGL2 через SDL3). Данные (30 МБ) упаковываются в `index.data`. Сеть в браузере отключена, доступны одиночная игра и кампании. Параметры страницы: `?fps` — счётчик кадров, `?args=--no-sound` — аргументы игры.

## Структура

| Путь | Что это |
|---|---|
| `btanks-code/trunk/engine` | движок: игровой цикл, мир, меню, ИИ, сеть, звук |
| `btanks-code/trunk/objects` | игровые объекты (плагин `bt_objects`) |
| `btanks-code/trunk/sdlx` | обёртка над SDL3; `Surface` — GPU-рендерер (blit → `SDL_RenderTexture`) |
| `btanks-code/trunk/clunk` | 3D-звук (SDL3 audio stream) |
| `btanks-code/trunk/mrt` | утилиты (строки, XML, сокеты, файлы) |
| `btanks-code/trunk/third_party/lua` | Lua 5.1.5 (MIT) |
| `btanks-code/trunk/data` | ресурсы: карты, тайлы, звуки, музыка |
| `web` | CMake-сборка под Emscripten и HTML-оболочка страницы |
| `btanks-code/trunk/editor` | редактор карт (пока на SDL 1.2, не собирается) |
| `btanks-code/branches` | старые ветки svn, для справки |
| `packaging/macos` | шаблон Info.plist и иконка для `make-app.sh` |

## Дорожная карта

- [x] Этап 1 — миграция на SDL3
- [x] Этап 2 — GPU-спрайтовый рендерер, широкоэкранный режим
- [x] Этап 2.1 — гигиена репозитория, воспроизводимая сборка без внешних префиксов
- [x] Этап 3 — рендер: атлас текстур (тайлы, пули, частицы), затухание объектов через alpha-mod на GPU
- [x] Этап 4 — фиксированный шаг симуляции 120 Гц (независимость от FPS), рабочий vsync
- [x] Этап 5 — `@rpath` для dylib, самодостаточный `.app` (`make-app.sh`), запуск из любого места
- [x] Этап 6 — Emscripten/WebGL: игра запускается и играется в браузере (синглплеер). Настройки и профиль сохраняются в IndexedDB. Осталось: мобильное управление, публикация
- [ ] Этап 7 — редактор карт на SDL3, интро-видео, мультиплеер в вебе (WebSocket)

## Лицензия

GPL v2 с исключением (см. `btanks-code/trunk/LICENSE` и `LICENSE.EXCEPTION`).
