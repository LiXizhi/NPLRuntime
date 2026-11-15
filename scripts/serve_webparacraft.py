"""
Local dev server for NPLRuntimeUnified WebParacraft (default <repo>/web).

Supports both the self-contained NPLRuntimeUnified/web shell (zero .data bundles,
direct local-disk mounting via ?searchroot=...&searchpath=...) and an external
webparacraft checkout if passed via --root.

URL conventions (matching webparacraft):
  - http://127.0.0.1:8088/npl/webparacraft/index.html?searchroot=C:/lxzsrc/ParaEngine/ParaWorld/&pid=530
  - http://127.0.0.1:8088/npl/webparacraft/keepwork/local.html?searchroot=C:/lxzsrc/ParaEngine/ParaWorld/&searchpath=script/&pid=530

Local disk endpoints (served directly by this HTTP server for speed, no MCP needed):
  - GET /fs/manifest?root=<searchroot>&searchpath=<searchpath>
  - GET /fs/file?root=<searchroot>&path=<relpath>
"""
import argparse
import base64
import gzip
import http.server
import json
import os
import posixpath
import re
import socket
import socketserver
import ssl
import threading
import urllib.parse
import webbrowser

PREFIX = "/npl/webparacraft/"
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
DEFAULT_WEB_ROOT = os.path.join(REPO_ROOT, "web")

EXCLUDED_DIR_NAMES = {
    ".git",
    ".svn",
    ".hg",
    ".vs",
    ".vscode",
    ".idea",
    ".venv",
    "venv",
    "node_modules",
    "__pycache__",
    "windowwebview2cache",
    "bin",
    "bin32",
    "bin64",
    "binnplruntime",
    "obj",
    "debug",
    "release",
    "x64",
    "win32build",
    "build",
    "out",
}

EXCLUDED_TOP_LEVEL_DIRS = {
    "web",
    "temp",
    "redist",
    "docs",
    "_doc",
    ".github",
    "_installfiles",
    "install",
    "installer",
    "nplruntime",
    "server",
    "client",
    "mobile",
    "screen shots",
    "_backup",
}

EXCLUDED_FILE_NAMES = {
    "log.txt",
    "assets.log",
    "paraengine.sig.tmp",
}

EXCLUDED_FILE_EXTS = {
    ".dmp",
    ".pdb",
    ".ilk",
    ".iobj",
    ".ipdb",
    ".obj",
    ".pch",
    ".suo",
    ".ncb",
    ".bak",
    ".db-shm",
    ".db-wal",
    ".db-journal",
}

EXCLUDED_DATABASE_SUBDIRS = {
    "npl",
    "personalpagestore",
}

EXCLUDED_DATABASE_FILES = {
    "localserver.db",
    "localserver.ver",
    "account.db",
    "userdata.db",
    "userdata.teen.db",
    "creator_profile.db",
    "userrobot.db",
    "localmarks.db",
    "map.db",
    "map1.db",
    "map2.db",
}

CORE_ROOT_DIRS = {
    "config",
    "npl_packages",
    "npl_mod",
    "packages",
    "database",
    "fonts",
    "mod",
    "_emptyworld",
    "texture",
    "character",
    "model",
    "shader",
    "shaders",
    "audio",
    "plugins",
}

CORE_ROOT_FILE_EXTS = {
    ".pkg",
    ".zip",
}

CORE_ROOT_FILES = {
    "assets_manifest.txt",
    "config.txt",
    "version.txt",
    "paraengine.sig",
}

VIRTUAL_CJK_FONT_REL = "fonts/DroidSansFallback.ttf"
VIRTUAL_TAHOMA_REL = "fonts/tahoma.ttf"
VIRTUAL_TAHOMA_BOLD_REL = "fonts/tahomabd.ttf"


def translate_cross_os_path(raw_path: str) -> str:
    """Allow C:/... URLs to work seamlessly on both Windows and WSL Python."""
    if not raw_path:
        return ""
    p = raw_path.strip().strip('"').strip("'")
    if os.name != "nt":
        m = re.match(r"^([A-Za-z]):[\\/](.*)$", p)
        if m:
            drive = m.group(1).lower()
            rest = m.group(2).replace("\\", "/")
            return f"/mnt/{drive}/{rest}"
    else:
        m = re.match(r"^/mnt/([A-Za-z])/(.*)$", p)
        if m:
            drive = m.group(1).upper()
            rest = m.group(2).replace("/", "\\")
            return f"{drive}:\\{rest}"
    return os.path.expanduser(p)


def detect_default_searchroot() -> str:
    candidates = []
    env_root = os.environ.get("PARACRAFT_SEARCHROOT") or os.environ.get("PARACRAFT_APP_DIR")
    if env_root:
        candidates.append(env_root)
    candidates.extend([
        "C:/lxzsrc/ParaEngine/ParaWorld",
        "C:/Paracraft",
        os.path.expanduser("~/paracraft"),
        "/mnt/c/lxzsrc/ParaEngine/ParaWorld",
        "/mnt/c/Paracraft",
    ])
    for cand in candidates:
        translated = translate_cross_os_path(cand)
        if translated and os.path.isdir(translated):
            if (
                os.path.isfile(os.path.join(translated, "main.pkg"))
                or os.path.isfile(os.path.join(translated, "main150727.pkg"))
                or os.path.isdir(os.path.join(translated, "script"))
                or os.path.isdir(os.path.join(translated, "npl_packages"))
            ):
                return os.path.realpath(translated)
    return ""


def detect_host_cjk_font(explicit_font: str = "") -> str:
    candidates = []
    if explicit_font:
        candidates.append(explicit_font)
    env_font = os.environ.get("PARACRAFT_FONT_FILE")
    if env_font:
        candidates.append(env_font)
    candidates.extend([
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/msyh.ttf",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/simsun.ttc",
        "/mnt/c/Windows/Fonts/msyh.ttc",
        "/mnt/c/Windows/Fonts/msyh.ttf",
        "/mnt/c/Windows/Fonts/simhei.ttf",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/STHeiti Light.ttc",
    ])
    for cand in candidates:
        translated = translate_cross_os_path(cand)
        if translated and os.path.isfile(translated):
            return os.path.realpath(translated)
    return ""


def _first_existing_font(candidates) -> str:
    for cand in candidates:
        if not cand:
            continue
        translated = translate_cross_os_path(cand)
        if translated and os.path.isfile(translated):
            return os.path.realpath(translated)
    return ""


def detect_host_latin_fonts(web_root: str = "") -> dict:
    """Resolve Tahoma (or the closest humanist sans-serif on this OS / web/fonts) for UI rendering."""
    bundled_reg = os.path.join(web_root, "fonts", "OpenSans-Regular.ttf") if web_root else ""
    bundled_bold = os.path.join(web_root, "fonts", "OpenSans-Semibold.ttf") if web_root else ""

    regular = _first_existing_font([
        os.environ.get("PARACRAFT_LATIN_FONT_FILE", ""),
        "C:/Windows/Fonts/tahoma.ttf",
        "/mnt/c/Windows/Fonts/tahoma.ttf",
        "/System/Library/Fonts/Supplemental/Tahoma.ttf",
        "/Library/Fonts/Tahoma.ttf",
        "C:/Windows/Fonts/verdana.ttf",
        "/mnt/c/Windows/Fonts/verdana.ttf",
        "/System/Library/Fonts/Supplemental/Verdana.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "/mnt/c/Windows/Fonts/segoeui.ttf",
        bundled_reg,
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    ])
    bold = _first_existing_font([
        os.environ.get("PARACRAFT_LATIN_BOLD_FONT_FILE", ""),
        "C:/Windows/Fonts/tahomabd.ttf",
        "/mnt/c/Windows/Fonts/tahomabd.ttf",
        "/System/Library/Fonts/Supplemental/Tahoma Bold.ttf",
        "/Library/Fonts/Tahoma Bold.ttf",
        "C:/Windows/Fonts/verdanab.ttf",
        "/mnt/c/Windows/Fonts/verdanab.ttf",
        "/System/Library/Fonts/Supplemental/Verdana Bold.ttf",
        "C:/Windows/Fonts/segoeuib.ttf",
        "/mnt/c/Windows/Fonts/segoeuib.ttf",
        bundled_bold,
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        regular,
    ])
    out = {}
    if regular:
        out[VIRTUAL_TAHOMA_REL] = regular
    if bold:
        out[VIRTUAL_TAHOMA_BOLD_REL] = bold
    return out


def is_loopback_client(addr: str) -> bool:
    return addr in ("127.0.0.1", "::1", "::ffff:127.0.0.1", "localhost")


def is_safe_searchroot(root_real: str) -> bool:
    if not root_real or not os.path.isdir(root_real):
        return False
    norm = root_real.replace("\\", "/").rstrip("/")
    # Block filesystem roots and OS system directories
    if re.match(r"^[A-Za-z]:$", norm) or norm in ("", "/", "/mnt", "/etc", "/proc", "/sys", "/usr", "/var"):
        return False
    lower = norm.lower()
    for forbidden in ("/windows", "c:/windows", "/system32"):
        if lower == forbidden or lower.startswith(forbidden + "/"):
            return False
    return True


def normalize_rel_path(raw_rel: str) -> str:
    rel = (raw_rel or "").replace("\\", "/").strip()
    while rel.startswith("/"):
        rel = rel[1:]
    if not rel:
        return ""
    norm = posixpath.normpath(rel)
    if norm in (".", "..") or norm.startswith("../") or "/../" in f"/{norm}/":
        return ""
    return norm


def resolve_path_case_insensitive(root_real: str, rel_norm: str) -> str:
    exact = os.path.join(root_real, *rel_norm.split("/"))
    if os.path.isfile(exact):
        real = os.path.realpath(exact)
        if os.path.commonpath([root_real, real]) == root_real:
            return real

    cur = root_real
    for part in rel_norm.split("/"):
        part_lower = part.lower()
        matched = None
        try:
            with os.scandir(cur) as it:
                for entry in it:
                    if entry.name.lower() == part_lower:
                        matched = entry.path
                        break
        except OSError:
            return ""
        if not matched:
            return ""
        cur = matched

    if os.path.isfile(cur):
        real = os.path.realpath(cur)
        if os.path.commonpath([root_real, real]) == root_real:
            return real
    return ""


def parse_searchpath_prefixes(raw_searchpath: str):
    if not raw_searchpath:
        return None
    parts = [
        p.replace("\\", "/").strip().strip("/")
        for p in re.split(r"[;,]", raw_searchpath)
    ]
    cleaned = [p for p in parts if p and p not in (".", "*")]
    if not cleaned:
        return None
    return cleaned


def is_allowed_database_rel(rel_lower: str) -> bool:
    if not rel_lower.startswith("database/"):
        return True
    rest = rel_lower[len("database/"):]
    slash = rest.find("/")
    if slash >= 0:
        sub = rest[:slash]
        if sub in EXCLUDED_DATABASE_SUBDIRS:
            return False
    fname = posixpath.basename(rest)
    if fname in EXCLUDED_DATABASE_FILES:
        return False
    if fname.endswith(("-shm", "-wal", "-journal")):
        return False
    return True


def is_core_runtime_rel(rel_posix: str) -> bool:
    slash = rel_posix.find("/")
    if slash < 0:
        lower = rel_posix.lower()
        if lower in CORE_ROOT_FILES:
            return True
        _, ext = os.path.splitext(lower)
        return ext in CORE_ROOT_FILE_EXTS
    rel_lower = rel_posix.lower()
    top = rel_lower[:slash]
    if top in CORE_ROOT_DIRS:
        return is_allowed_database_rel(rel_lower)
    if (
        rel_lower.startswith("worlds/designhouse/_emptyworld/")
        or rel_lower.startswith("worlds/designhouse/userworlds/")
        or rel_lower.startswith("worlds/templates/")
        or rel_lower.startswith("worlds/myworlds/flatgrassland/")
    ):
        return True
    return False


def matches_searchpath(rel_posix: str, prefixes) -> bool:
    rel_lower = rel_posix.lower()
    if not is_allowed_database_rel(rel_lower):
        return False
    if prefixes is None:
        return True
    if is_core_runtime_rel(rel_posix):
        return True
    for prefix in prefixes:
        p_lower = prefix.lower()
        if rel_lower == p_lower or rel_lower.startswith(p_lower + "/"):
            return True
    return False


def build_disk_manifest(root_real: str, prefixes, fallback_font_path: str, latin_fonts: dict = None):
    entries = []
    seen_lower = set()

    def walk_dir(abs_dir: str, rel_prefix: str, depth: int):
        try:
            scandir_it = os.scandir(abs_dir)
        except OSError:
            return
        with scandir_it as it:
            for entry in it:
                name = entry.name
                name_lower = name.lower()
                if name.startswith(".nplpackages-sync"):
                    continue
                try:
                    is_dir = entry.is_dir(follow_symlinks=False)
                except OSError:
                    continue

                rel_posix = f"{rel_prefix}/{name}" if rel_prefix else name

                if is_dir:
                    if name.startswith(".") or name_lower in EXCLUDED_DIR_NAMES:
                        continue
                    if depth == 0 and name_lower in EXCLUDED_TOP_LEVEL_DIRS:
                        continue
                    if depth == 1 and rel_prefix.lower() == "database" and name_lower in EXCLUDED_DATABASE_SUBDIRS:
                        continue
                    # Prune directory early if prefixes are active and neither inside nor ancestor of a prefix/core dir
                    if prefixes is not None and not is_core_runtime_rel(rel_posix + "/x"):
                        rel_lower = rel_posix.lower()
                        can_match = False
                        for p in prefixes:
                            pl = p.lower()
                            if rel_lower == pl or rel_lower.startswith(pl + "/") or pl.startswith(rel_lower + "/"):
                                can_match = True
                                break
                        if not can_match and not rel_lower.startswith("worlds"):
                            continue
                    walk_dir(entry.path, rel_posix, depth + 1)
                else:
                    if name_lower in EXCLUDED_FILE_NAMES:
                        continue
                    _, ext = os.path.splitext(name_lower)
                    if ext in EXCLUDED_FILE_EXTS:
                        continue
                    if not matches_searchpath(rel_posix, prefixes):
                        continue
                    try:
                        st = entry.stat(follow_symlinks=False)
                    except OSError:
                        continue
                    seen_lower.add(rel_posix.lower())
                    entries.append([rel_posix, st.st_size, int(st.st_mtime * 1000)])

    walk_dir(root_real, "", 0)

    virtual_fonts = []
    if fallback_font_path:
        virtual_fonts.append((VIRTUAL_CJK_FONT_REL, fallback_font_path))
    if latin_fonts:
        for v_rel, v_abs in latin_fonts.items():
            virtual_fonts.append((v_rel, v_abs))

    for v_rel, v_abs in virtual_fonts:
        if v_rel.lower() not in seen_lower and v_abs and os.path.isfile(v_abs):
            try:
                st = os.stat(v_abs)
                entries.append([v_rel, st.st_size, int(st.st_mtime * 1000)])
                seen_lower.add(v_rel.lower())
            except OSError:
                pass

    return entries


class ServerConfig:
    def __init__(self, root: str, engine_dir: str, default_searchroot: str, font_file: str, allow_lan_fs: bool, enable_coep: bool = False, latin_fonts: dict = None):
        self.root = root
        self.engine_dir = engine_dir
        self.default_searchroot = default_searchroot
        self.font_file = font_file
        self.latin_fonts = {k.lower(): v for k, v in (latin_fonts or {}).items()}
        self.latin_fonts_display = dict(latin_fonts or {})
        self.allow_lan_fs = allow_lan_fs
        self.enable_coep = enable_coep


class Handler(http.server.SimpleHTTPRequestHandler):
    server_config: ServerConfig = None

    extensions_map = {
        **http.server.SimpleHTTPRequestHandler.extensions_map,
        ".wasm": "application/wasm",
        ".js": "text/javascript",
        ".mjs": "text/javascript",
        ".css": "text/css",
        ".html": "text/html",
        ".json": "application/json",
        ".data": "application/octet-stream",
    }

    def end_headers(self):
        referer = self.headers.get("Referer", "")
        need_coep = (
            (self.server_config and self.server_config.enable_coep)
            or ("multithread=true" in self.path)
            or ("multithread=true" in referer)
        )
        if need_coep:
            self.send_header("Cross-Origin-Embedder-Policy", "credentialless")
            self.send_header("Cross-Origin-Opener-Policy", "same-origin")
            self.send_header("Cross-Origin-Resource-Policy", "cross-origin")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Cache-Control", "no-cache, no-store, must-revalidate")
        super().end_headers()

    def _send_json(self, status: int, payload: dict, compress_gzip: bool = False):
        raw = json.dumps(payload, ensure_ascii=False, separators=(",", ":")).encode("utf-8")
        accept_enc = self.headers.get("Accept-Encoding", "")
        use_gzip = compress_gzip and ("gzip" in accept_enc) and len(raw) > 1024
        body = gzip.compress(raw, compresslevel=5) if use_gzip else raw
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        if use_gzip:
            self.send_header("Content-Encoding", "gzip")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        if self.command != "HEAD":
            self.wfile.write(body)

    def _check_fs_access(self) -> bool:
        cfg = self.server_config
        client_ip = self.client_address[0] if self.client_address else ""
        if not cfg.allow_lan_fs and not is_loopback_client(client_ip):
            self._send_json(403, {"ok": False, "error": "Local disk /fs endpoints are restricted to loopback"})
            return False
        return True

    def _resolve_request_root(self, query_params) -> str:
        raw_root = (query_params.get("root", [""])[0] or "").strip()
        if raw_root:
            translated = translate_cross_os_path(raw_root)
            root_real = os.path.realpath(translated) if translated else ""
        else:
            root_real = self.server_config.default_searchroot
        if not is_safe_searchroot(root_real):
            return ""
        return root_real

    def _handle_fs_manifest(self, query_params):
        if not self._check_fs_access():
            return
        root_real = self._resolve_request_root(query_params)
        if not root_real:
            self._send_json(404, {
                "ok": False,
                "error": "Valid searchroot directory not found. Pass ?searchroot=C:/Paracraft/ in the URL.",
            })
            return
        raw_searchpath = (query_params.get("searchpath", [""])[0] or "").strip()
        prefixes = parse_searchpath_prefixes(raw_searchpath)
        entries = build_disk_manifest(
            root_real,
            prefixes,
            self.server_config.font_file,
            self.server_config.latin_fonts_display,
        )
        display_root = root_real.replace("\\", "/")
        if not display_root.endswith("/"):
            display_root += "/"
        self._send_json(200, {
            "ok": True,
            "root": display_root,
            "searchpath": raw_searchpath,
            "count": len(entries),
            "entries": entries,
        }, compress_gzip=True)

    def _handle_fs_file(self, query_params):
        if not self._check_fs_access():
            return
        root_real = self._resolve_request_root(query_params)
        if not root_real:
            self._send_json(404, {"ok": False, "error": "Invalid or missing root parameter"})
            return

        raw_rel = (query_params.get("path", [""])[0] or "").strip()
        rel_norm = normalize_rel_path(raw_rel)
        if not rel_norm:
            self._send_json(403, {"ok": False, "error": "Invalid relative path"})
            return

        target_file = resolve_path_case_insensitive(root_real, rel_norm)
        if not target_file:
            rel_lower = rel_norm.lower()
            if (
                rel_lower == VIRTUAL_CJK_FONT_REL.lower()
                and self.server_config.font_file
                and os.path.isfile(self.server_config.font_file)
            ):
                target_file = self.server_config.font_file
            elif rel_lower in self.server_config.latin_fonts:
                cand_font = self.server_config.latin_fonts[rel_lower]
                if cand_font and os.path.isfile(cand_font):
                    target_file = cand_font

        if not target_file or not os.path.isfile(target_file):
            self._send_json(404, {"ok": False, "error": f"File not found: {rel_norm}"})
            return

        try:
            file_size = os.path.getsize(target_file)
            range_header = self.headers.get("Range")
            start = 0
            end = file_size - 1
            status = 200

            if range_header and range_header.startswith("bytes=") and file_size > 0:
                m = re.match(r"^bytes=(\d*)-(\d*)$", range_header.strip())
                if m:
                    s_str, e_str = m.group(1), m.group(2)
                    if s_str:
                        start = int(s_str)
                        end = int(e_str) if e_str else file_size - 1
                    elif e_str:
                        suffix = int(e_str)
                        start = max(0, file_size - suffix)
                    if start > end or start >= file_size:
                        self.send_response(416)
                        self.send_header("Content-Range", f"bytes */{file_size}")
                        self.end_headers()
                        return
                    end = min(end, file_size - 1)
                    status = 206

            enc = (query_params.get("enc", [""])[0] or "").strip().lower()
            length = (end - start + 1) if file_size > 0 else 0
            if enc == "b64":
                raw_bytes = b""
                if length > 0:
                    with open(target_file, "rb") as f:
                        if start > 0:
                            f.seek(start)
                        raw_bytes = f.read(length)
                b64_bytes = base64.b64encode(raw_bytes) if raw_bytes else b""
                self.send_response(status)
                self.send_header("Content-Type", "text/plain; charset=ascii")
                self.send_header("Cache-Control", "no-store")
                self.send_header("Content-Length", str(len(b64_bytes)))
                self.end_headers()
                if self.command != "HEAD" and b64_bytes:
                    self.wfile.write(b64_bytes)
                return

            self.send_response(status)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Cache-Control", "no-store")
            self.send_header("Accept-Ranges", "bytes")
            if status == 206:
                self.send_header("Content-Range", f"bytes {start}-{end}/{file_size}")
            self.send_header("Content-Length", str(length))
            self.end_headers()

            if self.command != "HEAD" and length > 0:
                with open(target_file, "rb") as f:
                    if start > 0:
                        f.seek(start)
                    remaining = length
                    chunk_size = 256 * 1024
                    while remaining > 0:
                        chunk = f.read(min(chunk_size, remaining))
                        if not chunk:
                            break
                        self.wfile.write(chunk)
                        remaining -= len(chunk)
        except (BrokenPipeError, ConnectionResetError):
            pass
        except OSError as exc:
            self._send_json(500, {"ok": False, "error": str(exc)})

    def _handle_fs_log(self):
        if not self._check_fs_access():
            return
        try:
            length = int(self.headers.get("Content-Length", "0") or "0")
        except ValueError:
            length = 0
        raw = self.rfile.read(min(length, 512 * 1024)) if length > 0 else b""
        try:
            payload = json.loads(raw.decode("utf-8", errors="replace")) if raw else {}
            lines = payload.get("lines") if isinstance(payload, dict) else None
            if isinstance(lines, list):
                for line in lines:
                    print(f"[wasm] {line}", flush=True)
            elif raw:
                print(f"[wasm] {raw.decode('utf-8', errors='replace')}", flush=True)
        except Exception:
            if raw:
                print(f"[wasm] {raw.decode('utf-8', errors='replace')}", flush=True)
        self._send_json(200, {"ok": True})

    def _route_special_endpoints(self) -> bool:
        parsed = urllib.parse.urlsplit(self.path)
        url_path = parsed.path

        # Convenience redirect at root
        if url_path == "/" and self.command in ("GET", "HEAD"):
            qs = ("?" + parsed.query) if parsed.query else ""
            self.send_response(302)
            self.send_header("Location", f"{PREFIX}index.html{qs}")
            self.end_headers()
            return True

        # Support both /fs/* and /npl/webparacraft/fs/*
        stripped = url_path[len(PREFIX) - 1:] if url_path.startswith(PREFIX + "fs/") else url_path
        if stripped == "/fs/manifest" and self.command in ("GET", "HEAD"):
            self._handle_fs_manifest(urllib.parse.parse_qs(parsed.query, keep_blank_values=True))
            return True
        if stripped == "/fs/file" and self.command in ("GET", "HEAD"):
            self._handle_fs_file(urllib.parse.parse_qs(parsed.query, keep_blank_values=True))
            return True
        if stripped == "/fs/log" and self.command == "POST":
            self._handle_fs_log()
            return True
        return False

    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Access-Control-Allow-Methods", "GET, HEAD, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type, Range")
        self.end_headers()

    def do_POST(self):
        try:
            if self._route_special_endpoints():
                return
            self._send_json(404, {"ok": False, "error": "Unsupported POST endpoint"})
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
            pass

    def do_GET(self):
        try:
            if self._route_special_endpoints():
                return
            super().do_GET()
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
            pass

    def do_HEAD(self):
        try:
            if self._route_special_endpoints():
                return
            super().do_HEAD()
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
            pass

    def translate_path(self, path):
        url_path = urllib.parse.urlsplit(path).path
        if url_path in ("/ServiceWorker", "/ServiceWorker.js"):
            return super().translate_path("/ServiceWorker.js")
        if url_path.startswith(PREFIX):
            rel_url = "/" + url_path[len(PREFIX):]
        else:
            rel_url = url_path

        # Map /pid/<id> to /index.html
        if re.match(r"^/pid/\d+$", rel_url):
            rel_url = "/index.html"

        mapped = super().translate_path(rel_url)
        if (
            not os.path.exists(mapped)
            and self.server_config
            and self.server_config.engine_dir
            and rel_url.startswith("/auto_generate_files/")
        ):
            fname = os.path.basename(rel_url)
            alt = os.path.join(self.server_config.engine_dir, fname)
            if os.path.isfile(alt):
                return alt
        return mapped


def lan_ip():
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.connect(("10.255.255.255", 1))
            return s.getsockname()[0]
    except OSError:
        return None


def main():
    default_root = DEFAULT_WEB_ROOT if os.path.isfile(os.path.join(DEFAULT_WEB_ROOT, "index.html")) else os.path.expanduser("~/webparacraft")
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--root", default=default_root, help="Web host root directory (default: <repo>/web)")
    parser.add_argument("--engine-dir", default="", help="Optional CMake bin/ dir containing ParaCraftSingleThread.{js,wasm}")
    parser.add_argument("--searchroot", default="", help="Default local Paracraft directory when ?searchroot= is omitted in URL")
    parser.add_argument("--font-file", default="", help="Optional CJK .ttf/.ttc fallback when fonts/DroidSansFallback.ttf is absent")
    parser.add_argument("--allow-lan-fs", action="store_true", help="Allow LAN clients to read /fs/manifest and /fs/file")
    parser.add_argument("--coep", action="store_true", help="Send COOP/COEP headers unconditionally (auto-enabled when ?multithread=true)")
    parser.add_argument("--port", type=int, default=8088)
    parser.add_argument("--host", default="127.0.0.1", help="0.0.0.0 to allow LAN access")
    parser.add_argument("--open-url", default="", help="Optional URL to open in default browser once server is listening")
    parser.add_argument("--cert", help="TLS certificate (PEM) to serve https")
    parser.add_argument("--key", help="TLS private key (PEM)")
    args = parser.parse_args()

    root = os.path.abspath(translate_cross_os_path(args.root))
    if not os.path.isfile(os.path.join(root, "index.html")):
        raise SystemExit(f"{root} does not contain index.html")

    engine_dir = os.path.abspath(translate_cross_os_path(args.engine_dir)) if args.engine_dir else ""
    default_searchroot = (
        os.path.realpath(translate_cross_os_path(args.searchroot))
        if args.searchroot
        else detect_default_searchroot()
    )
    font_file = detect_host_cjk_font(args.font_file)
    latin_fonts = detect_host_latin_fonts(root)

    cfg = ServerConfig(
        root=root,
        engine_dir=engine_dir,
        default_searchroot=default_searchroot,
        font_file=font_file,
        allow_lan_fs=args.allow_lan_fs,
        enable_coep=args.coep,
        latin_fonts=latin_fonts,
    )

    class ConfiguredHandler(Handler):
        server_config = cfg

    def handler(*a, **k):
        return ConfiguredHandler(*a, directory=root, **k)

    socketserver.ThreadingTCPServer.allow_reuse_address = True
    with socketserver.ThreadingTCPServer((args.host, args.port), handler) as httpd:
        scheme = "http"
        if args.cert:
            context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
            context.load_cert_chain(args.cert, args.key)
            httpd.socket = context.wrap_socket(httpd.socket, server_side=True)
            scheme = "https"

        print(f"Serving WebParacraft shell from: {root}", flush=True)
        if engine_dir:
            print(f"  Engine fallback dir:   {engine_dir}", flush=True)
        if default_searchroot:
            sr_url = urllib.parse.quote(default_searchroot.replace("\\", "/") + "/", safe=":/")
            print(f"  Default searchroot:    {default_searchroot}", flush=True)
            print(f"  Local browser URL:     {scheme}://127.0.0.1:{args.port}{PREFIX}index.html?searchroot={sr_url}&pid=530", flush=True)
        else:
            print(f"  Local browser URL:     {scheme}://127.0.0.1:{args.port}{PREFIX}index.html?searchroot=C:/Paracraft/&pid=530", flush=True)
        if font_file:
            print(f"  CJK font fallback:     {font_file}", flush=True)
        if args.host != "127.0.0.1":
            ip = lan_ip()
            if ip:
                print(f"  LAN URL:               {scheme}://{ip}:{args.port}/index.html", flush=True)
        if args.open_url:
            print(f"  Opening browser:       {args.open_url}", flush=True)
            threading.Timer(0.3, lambda: webbrowser.open(args.open_url)).start()
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    main()
