"""Start a loopback Haqi test realm with the real Web client, town, NPCs and HUD."""

import argparse
import json
import secrets
import time
import webbrowser
from datetime import datetime
from http.cookies import SimpleCookie
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from threading import Lock, Thread
from urllib.parse import parse_qs, unquote, urlencode, urlsplit

from prepare_offline_assets import prepare

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
PREFIX = "/npl/webparacraft/"
BOOT = "script/apps/Aries/main_loop.lua"
SESSION = "haqi-test/session.json"
ARCHIVE = "haqi-test/plaza.zip"
LOGIN_PAGE = """<!doctype html><html lang="zh-CN"><meta charset="utf-8">
<title>哈奇本地测试服</title><main><h1>哈奇本地测试服</h1>
<p>临时测试角色 · 哈奇岛 · 36 个陪测角色</p>
<form method="post" action="/login"><button>使用测试角色登录哈奇镇</button></form></main></html>"""
POLICY = (
    "default-src 'self' blob: data:; script-src 'self' 'unsafe-inline' 'unsafe-eval' blob:; "
    "style-src 'self' 'unsafe-inline'; connect-src 'self' blob: data:; object-src 'none'"
)


def starter_item(guid, gsid, bag, copies):
    return {
        "guid": guid,
        "gsid": gsid,
        "bag": bag,
        "copies": copies,
        "position": 1,
        "clientdata": "",
        "serverdata": "",
        "obtaintime": "2020-01-01 12:00:00",
    }


class Session:
    def __init__(self):
        self.quests = {}
        self.hp = 100
        self.next_cast = 0
        self.bags = {
            12: [starter_item(1, 16001, 12, 3)],
            24: [starter_item(2, 22101, 24, 1)],
        }

    def snapshot(self, base):
        return {
            "nid": 101001,
            "nickname": "Offline Tester",
            "world": "61HaqiTown",
            "service": base,
            "server_time": datetime.now().strftime("%Y-%m-%d 12:00:00"),
            "quests": list(self.quests.values()),
            "pet": {
                "combatlel": 10,
                "combat_exp": 0,
                "level": 1,
                "health": 100,
                "stamina": 100,
            },
            "guests": [
                {"dx": 8 + (i % 6 - 2.5) * 1.8, "dz": (i // 6 - 2.5) * 1.8}
                for i in range(36)
            ],
        }


def serve(site, paraworld, archive, port, open_browser, check_output=None):
    base = f"http://127.0.0.1:{port}"
    entry = (
        PREFIX
        + "haqi1.html?"
        + urlencode(
            {
                "renderer": "opengl",
                "searchroot": "haqi-test",
                "searchpath": f"{BOOT},{SESSION},{ARCHIVE}",
                "mcp": base,
            }
        )
    )
    # Swap the login transport at the call site, retaining System.init and Aries_Init.
    # Check the source contract before serving any client if the upstream bootstrap changes.
    stock = (paraworld / BOOT).read_bytes()
    start = b"MyCompany.Aries.MainLogin:start(Aries_Init);"
    if stock.count(start) != 1:
        raise ValueError(
            "Haqi bootstrap has changed. Verify its login call before updating this fixture."
        )
    startup = b"\n".join(
        (HERE / "offline" / name).read_bytes()
        for name in ("plaza.lua", "quests.lua", "world.lua")
    )
    if check_output is not None:
        observer = (HERE / "offline/observe.lua").read_bytes()
        anchor = b"HaqiTest.Plaza.Start(profile)"
        startup = startup.replace(anchor, anchor + b"\n" + observer)
    bootstrap = stock.replace(
        start, b'System.options.version="kids"; Aries_Init();\n' + startup
    )
    sessions = {}
    lock = Lock()
    cookie_name = f"haqi_fixture_{port}"

    class Handler(SimpleHTTPRequestHandler):
        def send_bytes(self, body, content_type):
            self.send_response(200)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def send_json(self, data):
            self.send_bytes(json.dumps(data).encode(), "application/json")

        def end_headers(self):
            self.send_header("Content-Security-Policy", POLICY)
            self.send_header("Cache-Control", "no-store")
            super().end_headers()

        def key(self):
            cookie = SimpleCookie(self.headers.get("Cookie", "")).get(cookie_name)
            return cookie.value if cookie else None

        def do_POST(self):
            if self.headers.get("Origin", base) != base:
                self.send_error(403)
                return
            if self.path == "/login":
                key = secrets.token_urlsafe(24)
                with lock:
                    sessions.pop(self.key(), None)
                    sessions[key] = Session()
                self.send_response(303)
                self.send_header(
                    "Set-Cookie",
                    f"{cookie_name}={key}; Path=/; HttpOnly; SameSite=Strict",
                )
                self.send_header("Location", entry)
                self.end_headers()
                return
            if self.path == "/quests":
                length = self.headers.get("Content-Length", "")
                if (
                    not length.isdecimal()
                    or len(length) > 5
                    or not 0 < int(length) <= 16384
                ):
                    self.send_error(400)
                    return
                try:
                    quest = json.loads(self.rfile.read(int(length)))
                except (ValueError, UnicodeError):
                    quest = None
                if (
                    not isinstance(quest, dict)
                    or type(quest.get("id")) is not int
                    or quest.get("QuestState") not in (3, 5)
                ):
                    self.send_error(400)
                    return
                with lock:
                    session = sessions.get(self.key())
                    if not session:
                        self.send_error(403)
                        return
                    previous = session.quests.get(quest["id"])
                    if quest["QuestState"] == 5 and not previous:
                        self.send_error(409)
                        return
                    session.quests[quest["id"]] = quest
                self.send_json({"issuccess": True})
                return
            if self.path != "/training/cast":
                self.send_error(404)
                return
            if (
                self.headers.get("Content-Length") != "9"
                or self.rfile.read(9) != b"cast=fire"
            ):
                self.send_error(400)
                return
            with lock:
                session = sessions.get(self.key())
                if not session:
                    self.send_error(403)
                    return
                if time.monotonic() < session.next_cast:
                    self.send_error(429)
                    return
                session.hp = (session.hp or 100) - 25
                session.next_cast = time.monotonic() + 2
                result = {
                    "spell": "Fire_SingleAttack_Level1",
                    "hp": session.hp,
                    "damage": 25,
                }
            self.send_json(result)

        def do_GET(self):
            url = urlsplit(self.path)
            if url.path == "/":
                self.send_bytes(LOGIN_PAGE.encode(), "text/html; charset=utf-8")
                return
            if url.path == "/auth":
                session = sessions.get(self.key())
                if not session:
                    self.send_error(403)
                    return
                profile = session.snapshot(base)
                self.send_json(
                    {
                        "issuccess": True,
                        "nid": profile["nid"],
                        "userid": str(profile["nid"]),
                        "dt": profile["server_time"],
                        "sessionkey": "loopback-fixture",
                    }
                )
                return
            if url.path.startswith("/bags/"):
                bag = url.path.removeprefix("/bags/")
                if not bag.isascii() or not bag.isdecimal() or len(bag) > 6:
                    self.send_error(400)
                    return
                session = sessions.get(self.key())
                if not session:
                    self.send_error(403)
                    return
                self.send_json({"items": session.bags.get(int(bag), [])})
                return
            if url.path == "/fs/file":
                name = parse_qs(url.query).get("path", [""])[0]
                session = sessions.get(self.key())
                if not session:
                    self.send_error(403)
                    return
                if name == SESSION:
                    with lock:
                        snapshot = session.snapshot(base)
                    self.send_json(snapshot)
                    return
                if name == BOOT:
                    self.send_bytes(bootstrap, "application/octet-stream")
                    return
                if name != ARCHIVE:
                    self.send_error(404)
                    return
            super().do_GET()

        def translate_path(self, url):
            path = urlsplit(url)
            if path.path == "/fs/file" and parse_qs(path.query).get("path") == [
                ARCHIVE
            ]:
                return str(archive)
            if path.path.startswith(PREFIX):
                rel = Path(unquote(path.path[len(PREFIX) :]))
                if not rel.is_absolute() and all(
                    not x.startswith(".") for x in rel.parts
                ):
                    target = (site / rel).resolve()
                    if site in target.parents and target.is_file():
                        return str(target)
            return str(HERE / "nonexistent")

    with ThreadingHTTPServer(("127.0.0.1", port), Handler) as server:
        print(f"Haqi local test realm: {base}/", flush=True)
        if check_output is not None:
            from verify_offline_world import verify

            worker = Thread(target=server.serve_forever)
            worker.start()
            try:
                verify(base, check_output, "chrome")
            finally:
                server.shutdown()
                worker.join()
            return
        if open_browser:
            webbrowser.open(base + "/")
        server.serve_forever()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--web-root", type=Path, default=REPO.parent / "webparacraft")
    parser.add_argument(
        "--paraworld-root", type=Path, default=REPO.parent / "ParaEngine/paraworld"
    )
    parser.add_argument(
        "--archive", type=Path, default=REPO / "build/haqi-offline-test/town.zip"
    )
    parser.add_argument(
        "--native-cache", type=Path, default=REPO / "build/haqi-mac-runtime/temp/cache"
    )
    parser.add_argument(
        "--prepare", action="store_true", help="Build the public art pack and exit"
    )
    parser.add_argument(
        "--check", action="store_true", help="Run the Chrome regression and exit"
    )
    parser.add_argument("--port", type=int, default=8094)
    parser.add_argument("--no-browser", action="store_true")
    args = parser.parse_args()
    web, scripts, archive = (
        p.resolve() for p in (args.web_root, args.paraworld_root, args.archive)
    )
    if args.prepare:
        print(
            prepare(web, scripts, archive, args.native_cache), "public assets prepared"
        )
        return
    if not archive.is_file():
        parser.error(
            "Missing town archive. Run this launcher with --prepare once before going offline."
        )
    for file in (
        "haqi1.html",
        "auto_generate_files/ParaCraftSingleThread.js",
        "auto_generate_files/ParaCraftSingleThread.wasm",
    ):
        if not (web / file).is_file():
            parser.error(f"Missing {web / file}")
    serve(
        web,
        scripts,
        archive,
        args.port,
        not args.no_browser,
        archive.parent / "checks" if args.check else None,
    )


if __name__ == "__main__":
    main()
