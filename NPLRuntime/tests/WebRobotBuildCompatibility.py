"""Offline integration checks. Pass --protoc to a real host protoc 3.21.12."""
import argparse
import hashlib
import http.server
import io
import os
from pathlib import Path
import platform
import socket
import subprocess
import tempfile
import threading
import zipfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cmake", default="cmake")
    parser.add_argument("--protoc", type=Path, required=True)
    args = parser.parse_args()
    modules = Path(__file__).resolve().parent.parent / "cmake"
    requests = []
    payload = io.BytesIO()
    exe = "protoc.exe" if os.name == "nt" else "protoc"
    with zipfile.ZipFile(payload, "w") as archive:
        entry = zipfile.ZipInfo("bin/" + exe)
        entry.external_attr = 0o100755 << 16
        archive.writestr(entry, args.protoc.read_bytes())
    body = payload.getvalue()

    class Handler(http.server.BaseHTTPRequestHandler):
        def do_GET(self):
            requests.append(self.path)
            if len(requests) == 1:
                self.connection.shutdown(socket.SHUT_RDWR)
                self.connection.close()
                return
            self.send_response(200)
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def log_message(self, *unused):
            pass

    server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
        with tempfile.TemporaryDirectory(prefix="robot-build-compat-") as temporary:
            root = Path(temporary)
            script = root / "probe.cmake"

            def run(source, ok=True):
                script.write_text("cmake_minimum_required(VERSION 3.28)\n" + source, encoding="utf-8")
                env = dict(os.environ, NO_PROXY="127.0.0.1,localhost", no_proxy="127.0.0.1,localhost")
                result = subprocess.run([args.cmake, "-P", str(script)], capture_output=True, text=True, env=env)
                output = result.stdout + result.stderr
                assert (result.returncode == 0) == ok, output
                return output

            link = (modules / "WebRobotLinkFeatures.cmake").as_posix()
            run(f'''set(EMSCRIPTEN ON)
include("{link}")
if(NOT CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED)
 message(FATAL_ERROR "Missing old SDK fallback")
endif()
set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE "sdk-owned")
include("{link}")
if(NOT CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE STREQUAL "sdk-owned")
 message(FATAL_ERROR "Overwrote SDK feature")
endif()
''')
            run(f'''set(EMSCRIPTEN OFF)
include("{link}")
if(DEFINED CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED)
 message(FATAL_ERROR "Changed native platform")
endif()
''')
            source = root / "onnx"
            (source / "cmake").mkdir(parents=True)
            key = "win64" if os.name == "nt" else "mac_universal" if platform.system() == "Darwin" else "linux_aarch64" if platform.machine() in ("aarch64", "arm64") else "linux_x64"
            manifest = source / "cmake/deps.txt"
            url = f"http://127.0.0.1:{server.server_port}/protoc.zip"
            manifest.write_text(f"protoc_{key};{url};{hashlib.sha1(body).hexdigest()}\n")
            module = (modules / "WebRobotProtoc.cmake").as_posix()
            prefix = f'set(_robot_onnx_source "{source.as_posix()}")\nset(CMAKE_HOST_SYSTEM_PROCESSOR "{platform.machine()}")\n'
            custom = prefix + f'set(ONNX_CUSTOM_PROTOC_EXECUTABLE "{args.protoc.resolve().as_posix()}" CACHE FILEPATH "")\ninclude("{module}")\n'
            run(custom)
            assert not requests, "Custom protoc unexpectedly downloaded"
            automatic = prefix + f'set(CMAKE_BINARY_DIR "{(root / "build").as_posix()}")\ninclude("{module}")\n'
            output = run(automatic)
            assert "retrying protoc download with curl HTTP/1.1" in output, output
            count = len(requests)
            assert count >= 2
            run(automatic)
            assert len(requests) == count, "Cached protoc unexpectedly downloaded"
            manifest.write_text(f"protoc_{key};{url};{'0' * 40}\n")
            bad = automatic.replace('/build"', '/bad-checksum"')
            assert "checksum does not match" in run(bad, ok=False)
            assert "requires host protoc" in run(custom.replace(args.protoc.resolve().as_posix(), (root / "missing").as_posix()), ok=False)
            print("PASS: old SDK feature, SDK preservation, native isolation, custom protoc, HTTP/1.1 fallback, cache reuse, checksum and missing tool rejection")
    finally:
        server.shutdown()
        server.server_close()
        thread.join()


if __name__ == "__main__":
    main()
