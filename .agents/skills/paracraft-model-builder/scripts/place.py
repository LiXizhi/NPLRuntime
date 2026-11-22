"""Probe a GLB or place it at inspected absolute block coordinates via paracraft-cli."""
import argparse
import json
import math
from pathlib import Path
import re
import sys
import time
from urllib.request import Request, urlopen


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("identity", type=Path, help="saved get_creation_capabilities response")
    parser.add_argument("filename", help="relative filename returned by models.py fetch")
    parser.add_argument("--at", type=int, nargs=3, metavar=("X", "Y", "Z"))
    parser.add_argument("--scale", type=float, default=1)
    parser.add_argument("--facing", type=float, default=0, help="yaw in radians")
    parser.add_argument("--port", type=int, default=8099)
    parser.add_argument("--authoring-session", default="legacy", help="same authoringSession as the block job")
    args = parser.parse_args()
    if not re.fullmatch(r"blocktemplates/catalog/[a-z0-9_-]+/model\.glb", args.filename):
        parser.error("use the world-relative catalog filename")
    if not 0 < args.scale <= 10000 or not math.isfinite(args.facing):
        parser.error("supply a positive scale up to 10000 and finite yaw")
    envelope = json.loads(args.identity.read_text(encoding="utf-8"))
    identity = envelope["result"]["identity"]
    options = dict(identity=identity, filename=args.filename, scale=args.scale, facing=args.facing,
                   authoringSession=args.authoring_session)
    if args.at:
        options["at"] = args.at
    # Encode all user data as JSON, then as a single quoted Lua string.
    literal = json.dumps(json.dumps(options, ensure_ascii=False), ensure_ascii=False)
    code = "local p=commonlib.Json.Decode(" + literal + ")\n"
    code += Path(__file__).with_name("place.lua").read_text(encoding="utf-8")
    data = json.dumps({"action": "run_npl_code", "params": {"code": code}}).encode()
    request = Request(f"http://127.0.0.1:{args.port}/ajax/paracraft_cli", data=data,
                      headers={"Content-Type": "application/json"})
    deadline = time.monotonic() + 20
    while True:
        with urlopen(request, timeout=10) as response:
            reply = json.load(response)
        if not reply.get("ok"):
            raise ValueError(reply.get("error", str(reply)))
        result = reply["result"]["result"]
        if result["state"] not in ("loading", "instance_loading"):
            return result
        if time.monotonic() >= deadline:
            raise ValueError(f'{result["state"]}: native asset did not become ready. Inspect the carrier before retrying.')
        time.sleep(0.25)


if __name__ == "__main__":
    try:
        print(json.dumps(main(), indent=2, ensure_ascii=False))
    except (ValueError, KeyError, OSError) as error:
        sys.exit(str(error))
