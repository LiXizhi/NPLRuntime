"""Find and stage individual, self-contained GLBs for a local Paracraft world."""
import argparse
import hashlib
from itertools import islice
import json
import math
from pathlib import Path
import re
import struct
import sys
import tempfile
from urllib.parse import urlencode
from urllib.request import Request, urlopen

CODELS = "https://codels.download"
POLYFORK = "https://polyfork.dev"
POLY = "https://huggingface.co/datasets/rxr1/google-poly-glb"
REV = "df158247e0520e4c3b89f0b04e611f74e0eb69b4"
MIB = 1024 * 1024
DEFAULT_MODEL_BYTES = 64 * 1024


def download(url, limit):
    request = Request(url, headers={"User-Agent": "paracraft-model-builder/1.0"})
    with urlopen(request, timeout=45) as response:
        size = response.headers.get("Content-Length")
        if size and int(size) > limit:
            raise ValueError(f"download exceeds {limit} bytes")
        data = response.read(limit + 1)
    if len(data) > limit:
        raise ValueError(f"download exceeds {limit} bytes")
    return data


def poly_rows():
    # The index is 12 MB. Keep one immutable copy outside all worlds.
    index = Path(tempfile.gettempdir()) / f"paracraft-poly-{REV}.jsonl"
    if not index.exists():
        data = download(f"{POLY}/resolve/{REV}/metadata.jsonl", 20 * MIB)
        with tempfile.NamedTemporaryFile(dir=index.parent, delete=False) as tmp:
            tmp.write(data)
        Path(tmp.name).replace(index)
    with index.open(encoding="utf-8") as source:
        for line in source:
            yield json.loads(line)


def check_params(spec, pairs, typed=False):
    values = {}
    for pair in pairs:
        key, sep, value = pair.partition("=")
        if not sep or key not in spec or key in values:
            raise ValueError(f"unknown, duplicate or malformed parameter {pair!r}")
        rule = spec[key]
        kind = rule["type"]
        if kind in ("number", "int", "range"):
            number = int(value) if kind == "int" else float(value)
            valid = math.isfinite(number) and rule["min"] <= number <= rule["max"]
            if typed:
                value = number
        elif kind == "color":
            valid = bool(re.fullmatch(r"#[0-9a-fA-F]{6}", value))
        elif kind in ("boolean", "bool", "toggle"):
            valid = value in ("true", "false")
            if typed:
                value = value == "true"
        elif kind in ("select", "choice"):
            valid = value in rule["options"]
        else:
            raise ValueError(f"unsupported parameter type {kind}")
        if not valid:
            raise ValueError(f"{pair!r} is outside the published choices {rule}")
        values[key] = value
    return dict(sorted(values.items()))


def polyfork_model(slug, params):
    row = json.loads(download(f"{POLYFORK}/api/assets/{slug}", MIB))
    if not row.get("free"):
        raise ValueError("select a free low-poly model from the catalog")
    schema = {}
    if row.get("params_url"):
        schema = json.loads(download(row["params_url"], MIB))["params"]
    values = check_params(schema, params, typed=True)
    url = row["download"]["glb"]
    if values:
        query = urlencode({"p": json.dumps(values, separators=(",", ":"))})
        url = f"{POLYFORK}/cdn/{slug}-remix.glb?{query}"
    return dict(title=row["title"], author="Polyfork", page=row["page"],
                license="CC-BY-4.0", license_url="https://creativecommons.org/licenses/by/4.0/",
                credit=row["credit"], provider_license=f"{POLYFORK}/licensing",
                changes="Baked with the listed parameters." if values else "Original low-poly mesh.",
                schema=schema, parameters=values, url=url)


def describe(catalog, slug, params):
    if not re.fullmatch(r"[a-z0-9][a-z0-9_-]{0,127}", slug):
        raise ValueError("use a model ID from search")
    if catalog == "polyfork":
        return polyfork_model(slug, params)
    if catalog == "poly":
        if params:
            raise ValueError("Poly assets support instance transforms only")
        row = next((r for r in poly_rows() if r["id"] == slug), None)
        if not row:
            raise ValueError(f"unknown Poly ID {slug}")
        return dict(title=row["title"], author=row["authors"], page=row["source_url"],
                    license=row["license"], license_url=row["license_url"],
                    url=f'{POLY}/resolve/{REV}/{row["files"]["engine"]}',
                    bytes=row["engine_bytes"], changes="CodePawl engine-tier conversion; copied as supplied.")
    row = json.loads(download(f"{CODELS}/api/v1/models/{slug}", MIB))
    spec = row.get("params") or {}
    values = check_params(spec, params)
    query = ("?" + urlencode(values)) if values else ""
    return dict(title=row["title"], page=row["urls"]["page"], author=row.get("author"),
                license=row["license"], version=row["version"], parameters=values,
                schema=spec, conventions=row.get("conventions"),
                url=f"{CODELS}/m/{slug}.glb{query}", generator=f"{CODELS}/m/{slug}.js{query}")


def mesh_stats(data):
    if len(data) < 20:
        raise ValueError("incomplete GLB")
    magic, version, length, chunk, kind = struct.unpack_from("<4sIII4s", data)
    if (magic, version, length, kind) != (b"glTF", 2, len(data), b"JSON") or chunk > length - 20:
        raise ValueError("expected a complete GLB 2 file")
    doc = json.loads(data[20:20 + chunk])
    if not any(mesh.get("primitives") for mesh in doc.get("meshes", [])):
        raise ValueError("model contains no mesh")
    for resource in doc.get("buffers", []) + doc.get("images", []):
        if "uri" in resource and not resource["uri"].startswith("data:"):
            raise ValueError("model requires external files")
    embedded = [buf for buf in doc.get("buffers", []) if "uri" not in buf]
    if embedded:
        offset = 20 + chunk
        if len(embedded) != 1 or offset + 8 > length:
            raise ValueError("missing embedded geometry buffer")
        size, kind = struct.unpack_from("<I4s", data, offset)
        if kind != b"BIN\x00" or size > length - offset - 8 or embedded[0]["byteLength"] > size:
            raise ValueError("incomplete embedded geometry buffer")
    triangles = 0
    primitives = 0
    for mesh in doc["meshes"]:
        for part in mesh["primitives"]:
            primitives += 1
            mode = part.get("mode", 4)
            if mode in (4, 5, 6):
                index = part["indices"] if "indices" in part else part["attributes"]["POSITION"]
                count = doc["accessors"][index]["count"]
                triangles += count // 3 if mode == 4 else max(0, count - 2)
    return {"bytes": len(data), "triangles": triangles, "primitives": primitives}


def read_model(args, info):
    if args.max_bytes <= 0 or info.get("bytes", 0) > args.max_bytes:
        raise ValueError(f"model exceeds the {args.max_bytes}-byte budget")
    data = download(info["url"], args.max_bytes)
    if "bytes" in info and info["bytes"] != len(data):
        raise ValueError("model length differs from the catalog")
    return data, mesh_stats(data)


def stage(args, info):
    world = args.world.resolve(strict=True)
    if not (world / "tag.xml").is_file():
        raise ValueError("expected an existing local world with tag.xml")
    data, stats = read_model(args, info)
    info.update(bytes=len(data), sha256=hashlib.sha256(data).hexdigest())
    info["catalog"], info["id"] = args.catalog, args.model
    receipt = (json.dumps(info, indent=2, ensure_ascii=False) + "\n").encode()
    # Include source and parameters in the identity, even for identical geometry.
    key = hashlib.sha256(receipt).hexdigest()[:16]
    parent = world / "blocktemplates" / "catalog"
    if world not in parent.resolve().parents:
        raise ValueError("asset directory leaves the world")
    parent.mkdir(parents=True, exist_ok=True)
    dest = parent / f"{args.catalog}-{args.model}-{key}"
    with tempfile.TemporaryDirectory(dir=parent) as tmp:
        bundle = Path(tmp) / "bundle"
        bundle.mkdir()
        (bundle / "model.glb").write_bytes(data)
        (bundle / "source.json").write_bytes(receipt)
        try:
            bundle.rename(dest)
        except OSError:
            if (dest.is_symlink() or (dest / "source.json").read_bytes() != receipt
                    or (dest / "model.glb").read_bytes() != data):
                raise
    return {"filename": (dest / "model.glb").relative_to(world).as_posix(),
            **stats, "sha256": info["sha256"]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    actions = parser.add_subparsers(dest="action", required=True)
    for action in ("search", "info", "measure", "fetch"):
        cmd = actions.add_parser(action)
        cmd.add_argument("catalog", choices=("polyfork", "codels", "poly"))
        cmd.add_argument("model", help="search terms or exact model ID")
        if action == "fetch":
            cmd.add_argument("--world", type=Path, required=True)
        if action in ("measure", "fetch"):
            cmd.add_argument("--param", action="append", default=[], metavar="KEY=VALUE")
            cmd.add_argument("--max-bytes", type=int, default=DEFAULT_MODEL_BYTES,
                             help="maximum GLB size in bytes (default: 65536)")
    args = parser.parse_args()
    if args.action == "search":
        if args.catalog == "polyfork":
            query = urlencode({"q": args.model, "free": 1, "per_page": 100, "sort": "tris"})
            items = json.loads(download(f"{POLYFORK}/api/assets?{query}", MIB))["assets"]
            terms = args.model.casefold().split()
            items.sort(key=lambda row: (not all(t in row["title"].casefold() for t in terms),
                                        row["triangles"]))
            return [{"id": r["id"], "title": r["title"], "parametric": r["remixable"],
                     "triangles": r["triangles"], "size_m": r.get("size_m")} for r in items[:20]]
        if args.catalog == "codels":
            url = f"{CODELS}/api/v1/models?" + urlencode({"q": args.model})
            items = json.loads(download(url, MIB))["items"]
            return [{"id": r["slug"], "title": r["title"], "parametric": r["parametric"]} for r in items[:20]]
        terms = args.model.casefold().split()
        matches = (r for r in poly_rows() if all(term in
                   f'{r["title"]} {r["kind"]} {r["id"]}'.casefold() for term in terms))
        return [{"id": r["id"], "title": r["title"], "bytes": r["engine_bytes"],
                 "author": r["authors"]} for r in islice(matches, 20)]
    info = describe(args.catalog, args.model, getattr(args, "param", []))
    if args.action == "measure":
        return read_model(args, info)[1]
    return stage(args, info) if args.action == "fetch" else info


if __name__ == "__main__":
    try:
        print(json.dumps(main(), indent=2, ensure_ascii=False))
    except (OSError, ValueError, KeyError) as error:
        sys.exit(str(error))
