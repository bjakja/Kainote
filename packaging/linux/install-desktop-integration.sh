#!/bin/sh
# Registers this Kainote directory with the desktop: menu entry, file
# associations and icons, all under $XDG_DATA_HOME. No root needed, and
# nothing outside that directory is touched.
#
# The tarball is a portable tree, so Exec= has to name this copy's absolute
# path rather than expecting kainote on PATH.

set -eu

here=$(cd "$(dirname "$0")" && pwd)
data_home=${XDG_DATA_HOME:-$HOME/.local/share}
app_id=io.github.bjakja.Kainote

usage()
{
	echo "usage: $0 [--uninstall]" >&2
	exit 2
}

refresh()
{
	# Each is optional; a desktop without them still works after a re-login.
	command -v update-desktop-database >/dev/null 2>&1 &&
		update-desktop-database "$data_home/applications" >/dev/null 2>&1 || true
	command -v update-mime-database >/dev/null 2>&1 &&
		update-mime-database "$data_home/mime" >/dev/null 2>&1 || true
	command -v gtk-update-icon-cache >/dev/null 2>&1 &&
		gtk-update-icon-cache -f -t "$data_home/icons/hicolor" >/dev/null 2>&1 || true
}

uninstall()
{
	rm -f "$data_home/applications/$app_id.desktop"
	rm -f "$data_home/metainfo/$app_id.metainfo.xml"
	rm -f "$data_home/mime/packages/kainote.xml"
	find "$data_home/icons/hicolor" -name 'kainote.png' -delete 2>/dev/null || true
	for name in text-x-ssa application-x-subrip text-x-microdvd; do
		find "$data_home/icons/hicolor" -name "$name.png" -delete 2>/dev/null || true
	done
	refresh
	echo "Removed Kainote desktop integration from $data_home"
}

case "${1-}" in
	--uninstall) uninstall; exit 0 ;;
	'') ;;
	*) usage ;;
esac

if [ ! -x "$here/kainote" ]; then
	echo "error: no kainote executable next to $0" >&2
	exit 1
fi

mkdir -p "$data_home/applications" "$data_home/metainfo" \
	"$data_home/mime/packages" "$data_home/icons"

# Desktop entry Exec= splits unquoted spaces into arguments. Quote the path and
# escape characters that are special inside its quoted argument.
exec_path=$(printf '%s' "$here/kainote" |
	sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/\$/\\$/g' -e 's/`/\\`/g')
while IFS= read -r line || [ -n "$line" ]; do
	case "$line" in
		Exec=*) printf 'Exec="%s" %%f\n' "$exec_path" ;;
		*) printf '%s\n' "$line" ;;
	esac
done < "$here/share/applications/$app_id.desktop" \
	> "$data_home/applications/$app_id.desktop"

cp "$here/share/metainfo/$app_id.metainfo.xml" "$data_home/metainfo/"
cp "$here/share/mime/packages/kainote.xml" "$data_home/mime/packages/"
cp -r "$here/share/icons/hicolor" "$data_home/icons/"

refresh

echo "Installed Kainote desktop integration into $data_home"
echo "Run '$0 --uninstall' to remove it."
