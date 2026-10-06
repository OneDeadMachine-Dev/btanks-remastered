#!/bin/zsh
# Собирает самодостаточный "Battle Tanks.app" в dist/ (macOS, Apple Silicon).
# Внутри: бинарник, все dylib (включая зависимости из Homebrew), данные игры.
# Подпись ad-hoc: приложение запускается на этом Mac; для раздачи другим нужен
# Developer ID и нотаризация (см. README).
set -e
ROOT="${0:A:h}"
TRUNK="$ROOT/btanks-code/trunk"
APP="$ROOT/dist/Battle Tanks.app"
FW="$APP/Contents/Frameworks"
RES="$APP/Contents/Resources"
MACOS="$APP/Contents/MacOS"

"$ROOT/build-mac.sh" >/dev/null

rm -rf "$APP"
mkdir -p "$FW" "$RES" "$MACOS"

cp "$TRUNK/btanks" "$MACOS/btanks"
for entry in mrt:mrt sdlx:sdlx clunk:clunk btanks_engine:engine bt_objects:objects; do
	cp "$TRUNK/build/release/${entry#*:}/lib${entry%%:*}.dylib" "$FW/"
done
rsync -a --exclude='*.svn*' --exclude='.DS_Store' "$TRUNK/data" "$RES/"
cp "$ROOT/packaging/macos/btanks.icns" "$RES/"

sed -e 's|<string>btanks-launcher</string>|<string>btanks</string>|' \
	"$ROOT/packaging/macos/Info.plist" > "$APP/Contents/Info.plist"

# --- зависимости: рекурсивно копируем всё, что не системное, и переписываем пути ---
typeset -A origin   # имя библиотеки -> исходный файл (чтобы разрешать её @rpath-зависимости)

# находит файл для "@rpath/name" по LC_RPATH исходной библиотеки (или в Homebrew)
resolve_rpath() {
	local name="$1" from="$2" rp cand
	if [[ -n "$from" ]]; then
		for rp in $(otool -l "$from" | awk '/LC_RPATH/{f=1} f&&/ path /{print $2; f=0}'); do
			cand="${rp/@loader_path/${from:h}}/$name"
			[[ -f "$cand" ]] && { echo "$cand"; return; }
		done
	fi
	for cand in "$(brew --prefix)/lib/$name" ${(f)"$(ls -d $(brew --prefix)/opt/*/lib/$name 2>/dev/null)"}; do
		[[ -f "$cand" ]] && { echo "$cand"; return; }
	done
}

bundle_deps() {
	local file="$1" dep base src
	for dep in $(otool -L "$file" | tail -n +2 | awk '{print $1}'); do
		case "$dep" in
			/usr/lib/*|/System/*|@executable_path/*|@loader_path/*) continue ;;
			@rpath/*) base="${dep#@rpath/}"; [[ -f "$FW/$base" ]] && continue
				src="$(resolve_rpath "$base" "${origin[${file:t}]}")" ;;
			*) base="${dep:t}"; src="$dep"; install_name_tool -change "$dep" "@rpath/$base" "$file" 2>/dev/null ;;
		esac
		if [[ ! -f "$FW/$base" ]]; then
			[[ -n "$src" ]] || { echo "не найдена зависимость $dep ($file)"; exit 1; }
			cp -L "$src" "$FW/$base"
			chmod u+w "$FW/$base"
			origin[$base]="${src:A}"
			install_name_tool -id "@rpath/$base" "$FW/$base" 2>/dev/null
			bundle_deps "$FW/$base"
		fi
	done
}
for f in "$MACOS/btanks" "$FW"/*.dylib(N); do
	bundle_deps "$f"
done
# убрать rpath, ведущие наружу (в Homebrew): приложение должно жить само по себе
for f in "$MACOS/btanks" "$FW"/*.dylib; do
	for rp in $(otool -l "$f" | awk '/LC_RPATH/{f=1} f&&/ path /{print $2; f=0}'); do
		case "$rp" in /opt/*|/usr/local/*) install_name_tool -delete_rpath "$rp" "$f" 2>/dev/null ;; esac
	done
done

codesign --force --deep --sign - "$APP" 2>&1 | grep -v "replacing existing signature" || true
echo "готово: $APP ($(du -sh "$APP" | cut -f1))"
