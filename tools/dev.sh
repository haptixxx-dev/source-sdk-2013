#!/usr/bin/env bash
# Cascade developer helper — build, run, debug and tail logs on Linux.
#
# Copyright (C) 2026 Haptixxx
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# Note: this script is owner-authored tooling and lives outside src/. The engine
# and SDK sources under src/ remain under the SOURCE 1 SDK LICENSE.

set -euo pipefail

REPO="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
MOD="cascade"
MOD_DIR="$REPO/game/$MOD"
LAUNCHER="$REPO/game/${MOD}_linux64"
STEAM_ROOT="${STEAM_ROOT:-$HOME/.local/share/Steam}"
STEAM_COMMON="${STEAM_COMMON:-$STEAM_ROOT/steamapps/common}"
SDK_DIR="$STEAM_COMMON/Source SDK Base 2013 Multiplayer"
HL2_DIR="$STEAM_COMMON/Half-Life 2/hl2"
TESTMAPS_DIR="$SDK_DIR/sourcetest"

# Steam libraries can live anywhere; find the Steam Linux Runtime 3.0 (sniper) that app 243750 needs.
find_sniper() {
	local lib
	for lib in "$STEAM_ROOT" $(sed -n 's/.*"path"[[:space:]]*"\([^"]*\)".*/\1/p' "$STEAM_ROOT/steamapps/libraryfolders.vdf" 2>/dev/null); do
		[[ -x "$lib/steamapps/common/SteamLinuxRuntime_sniper/run" ]] && { echo "$lib/steamapps/common/SteamLinuxRuntime_sniper/run"; return 0; }
	done
	return 1
}

usage() {
	cat <<USAGE
usage: tools/dev.sh <command> [args]

  build [debug|release]   Build everything in the Steam Runtime container (default: release).
                          Deletes the cached VPC solution first if any .vpc/.vgc is newer than it.
  run [--testmaps] [--hl2maps] [--steam|--direct] [args]
                          Launch $MOD windowed with -dev -console -novid -condebug inside the
                          Steam Linux Runtime 3.0 (sniper) container, the way Steam launches app 243750.
                          Extra args pass through, e.g. run --testmaps +map test_hardware
                          --testmaps mounts SDK Base's sourcetest/ (test_hardware, background01 maps)
                          --hl2maps  mounts the Half-Life 2 (app 220) content dir (campaign maps)
                          --steam    use 'steam -applaunch 243750 -game ...' instead of the runtime directly
                          --direct   exec game/${MOD}_linux64 on the host. Needs the engine's runtime
                                     libs installed (e.g. libcurl-gnutls.so.4); usually fails on Arch
  attach-gdb              Attach gdb to the running engine process (build debug first).
  log                     Tail $MOD_DIR/console.log
  path                    Print the launcher, mod dir and engine dir that will be used.
USAGE
}

vpc_stale() {
	local sln="$REPO/src/_vpc_/ninja/sdk_everything_$1.ninja"
	[[ -e "$sln" ]] || return 0
	[[ -n "$(find "$REPO/src" -newer "$sln" \( -name '*.vpc' -o -name '*.vgc' \) -print -quit)" ]]
}

cmd_build() {
	local mode="${1:-release}"
	case "$mode" in debug|release) ;; *) echo "build: expected debug|release" >&2; exit 2 ;; esac
	if vpc_stale "$mode"; then
		echo "[dev.sh] VPC scripts changed — regenerating $mode solution"
		rm -f "$REPO/src/_vpc_/ninja/sdk_everything_$mode.ninja"
	fi
	( cd "$REPO/src" && ./buildallprojects "$mode" )
}

cmd_run() {
	local extra=() mode=runtime
	while [[ $# -gt 0 ]]; do
		case "$1" in
			--testmaps)
				[[ -d "$TESTMAPS_DIR" ]] || { echo "run: $TESTMAPS_DIR not found" >&2; exit 1; }
				extra+=( -insert_search_path "$TESTMAPS_DIR" ) ;;
			--hl2maps)
				[[ -d "$HL2_DIR" ]] || { echo "run: Half-Life 2 not found at $HL2_DIR" >&2; exit 1; }
				extra+=( -insert_search_path "$HL2_DIR" ) ;;
			--steam)  mode=steam ;;
			--direct) mode=direct ;;
			*) extra+=( "$1" ) ;;
		esac
		shift
	done
	[[ -f "$MOD_DIR/bin/linux64/client.so" ]] || { echo "run: $MOD_DIR/bin/linux64/client.so missing — run 'tools/dev.sh build' first" >&2; exit 1; }
	local common=( -dev -console -novid -condebug -windowed -w 1280 -h 720 )
	case "$mode" in
		direct)
			[[ -x "$LAUNCHER" ]] || { echo "run: launcher missing — run 'tools/dev.sh build' first" >&2; exit 1; }
			cd "$REPO/game" && exec "$LAUNCHER" "${common[@]}" "${extra[@]}" ;;
		steam)
			command -v steam >/dev/null || { echo "run: 'steam' not on PATH" >&2; exit 1; }
			exec steam -applaunch 243750 -game "$MOD_DIR" "${common[@]}" "${extra[@]}" ;;
		runtime)
			local slr
			slr="$(find_sniper)" || { echo "run: SteamLinuxRuntime_sniper not installed — launch app 243750 once from Steam to fetch it, or use --steam" >&2; exit 1; }
			[[ -x "$SDK_DIR/hl2.sh" ]] || { echo "run: $SDK_DIR/hl2.sh missing — install Source SDK Base 2013 Multiplayer" >&2; exit 1; }
			cd "$REPO/game" && exec "$slr" -- "$SDK_DIR/hl2.sh" -game "$MOD_DIR" "${common[@]}" "${extra[@]}" ;;
	esac
}

cmd_attach_gdb() {
	local pid
	pid="$(pgrep -n -f 'hl2_linux64' || true)"
	[[ -n "$pid" ]] || { echo "attach-gdb: no hl2_linux64 process" >&2; exit 1; }
	exec gdb -p "$pid" \
		-ex "set solib-search-path $MOD_DIR/bin/linux64" \
		-ex "handle SIGPIPE nostop noprint" \
		-ex "handle SIG32 nostop noprint"
}

cmd_log() {
	exec tail -n 50 -F "$MOD_DIR/console.log"
}

cmd_path() {
	echo "launcher: $LAUNCHER"
	echo "mod dir:  $MOD_DIR"
	echo "engine:   $SDK_DIR"
	echo "runtime:  $(find_sniper || echo '<SteamLinuxRuntime_sniper not found>')"
	echo "testmaps: $TESTMAPS_DIR"
}

case "${1:-}" in
	build)      shift; cmd_build "$@" ;;
	run)        shift; cmd_run "$@" ;;
	attach-gdb) shift; cmd_attach_gdb "$@" ;;
	log)        shift; cmd_log "$@" ;;
	path)       shift; cmd_path "$@" ;;
	*)          usage; exit 2 ;;
esac
