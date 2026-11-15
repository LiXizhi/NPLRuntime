// Local-disk MEMFS driver for NPLRuntimeUnified WebParacraft.
// Reuses WebParacraft's existing ?searchroot=...&searchpath=... URL parameters and
// /fs/file?root=...&path=... endpoint convention, served directly by the local
// HTTP server (serve_webparacraft.py) without needing Keepwork MCP (:8089) or .data bundles.

(function () {
    const DEFERRED_MOUNT_TOP_DIRS = new Set(["apps", "worlds", "database"]);

    const BOOT_PRELOAD_EXACT = new Set([
        "main.pkg",
        "main150727.pkg",
        "main_mobile_res.pkg",
        "assets_manifest.txt",
        "config.txt",
        "version.txt",
        "paraengine.sig",
        "fonts/droidsansfallback.ttf",
        "npl_packages/paracraftbuildinmod.zip",
        "database/characters.db",
        "config/characters.db",
    ]);

    function getQueryParam(key) {
        try {
            const params = new URLSearchParams(window.location.search || "");
            return (params.get(key) || "").trim();
        } catch (e) {
            return "";
        }
    }

    function shouldPrestageAtBoot(relPath, size) {
        const lower = relPath.toLowerCase();
        if (BOOT_PRELOAD_EXACT.has(lower)) return true;
        if (lower.startsWith("config/") && lower.indexOf("/", 7) < 0 && size <= 4 * 1024 * 1024) return true;
        if (lower.endsWith(".pkg") && lower.indexOf("/") < 0) return true;
        if (lower.startsWith("npl_packages/") && lower.indexOf("/", 13) < 0 && lower.endsWith(".zip") && size <= 64 * 1024 * 1024) return true;
        return false;
    }

    function registerCiChild(parent, name) {
        if (!parent || !name) return;
        if (!parent._ciMap) parent._ciMap = Object.create(null);
        parent._ciMap[name.toLowerCase()] = name;
    }

    function resolveChildNode(parent, name) {
        if (!parent || !parent.contents) return null;
        if (Object.prototype.hasOwnProperty.call(parent.contents, name)) {
            return parent.contents[name];
        }
        const mapped = parent._ciMap && parent._ciMap[name.toLowerCase()];
        if (mapped && Object.prototype.hasOwnProperty.call(parent.contents, mapped)) {
            return parent.contents[mapped];
        }
        return null;
    }

    const state = {
        installed: false,
        patchedMemfs: false,
        fsOrigin: window.location.origin,
        resolvedRoot: "",
        searchpath: "",
        deferredEntries: [],
        stats: {
            manifestEntries: 0,
            prestagedFiles: 0,
            prestagedBytes: 0,
            lazyFetches: 0,
            lazyBytes: 0,
            lazyErrors: 0,
        },
    };

    window.__localDiskStats = state.stats;

    function buildFileUrl(relPath) {
        return (
            state.fsOrigin +
            "/fs/file?root=" +
            encodeURIComponent(state.resolvedRoot) +
            "&path=" +
            encodeURIComponent(relPath)
        );
    }

    function syncFetchBytes(relPath) {
        const url = buildFileUrl(relPath);
        const xhr = new XMLHttpRequest();
        xhr.open("GET", url, false);
        xhr.overrideMimeType("text/plain; charset=x-user-defined");
        xhr.send(null);
        if (xhr.status !== 200) {
            throw new Error("HTTP " + xhr.status + " for " + relPath);
        }
        const text = xhr.responseText || "";
        const bytes = new Uint8Array(text.length);
        for (let i = 0; i < text.length; i++) {
            bytes[i] = text.charCodeAt(i) & 0xff;
        }
        return bytes;
    }

    function ensureMaterialized(node) {
        if (!node || !node._diskRelPath || node._materialized) return;
        const relPath = node._diskRelPath;
        try {
            const bytes = syncFetchBytes(relPath);
            node.contents = bytes;
            node.usedBytes = bytes.byteLength;
            node._diskSize = bytes.byteLength;
            node._materialized = true;
            state.stats.lazyFetches += 1;
            state.stats.lazyBytes += bytes.byteLength;
        } catch (err) {
            state.stats.lazyErrors += 1;
            console.error("[local_disk_fs] Failed to materialize", relPath, err);
            node.contents = new Uint8Array(0);
            node.usedBytes = 0;
            node._diskSize = 0;
            node._materialized = true;
        }
    }

    function markNodeOverwritten(node) {
        if (!node || !node._diskRelPath || node._materialized) return;
        node._diskRelPath = null;
        node._diskSize = 0;
        node._materialized = true;
        node.contents = new Uint8Array(0);
        node.usedBytes = 0;
    }

    function patchMemfsOnce() {
        if (state.patchedMemfs) return;
        if (typeof FS === "undefined" || typeof MEMFS === "undefined") {
            console.warn("[local_disk_fs] FS/MEMFS globals unavailable");
            return;
        }
        state.patchedMemfs = true;

        // Ensure MEMFS.ops_table is initialized before patching method references
        if (!MEMFS.ops_table && FS.root) {
            try {
                const probe = MEMFS.createNode(FS.root, ".__memfs_probe__", 33206, 0);
                delete FS.root.contents[".__memfs_probe__"];
                void probe;
            } catch (e) {
                // ignore
            }
        }

        const origCreateNode = MEMFS.createNode;
        MEMFS.createNode = function (parent, name, mode, dev) {
            const node = origCreateNode.call(this, parent, name, mode, dev);
            if (parent && name) {
                registerCiChild(parent, name);
            }
            return node;
        };

        const origGetFileData = MEMFS.getFileDataAsTypedArray;
        if (typeof origGetFileData === "function") {
            MEMFS.getFileDataAsTypedArray = function (node) {
                ensureMaterialized(node);
                return origGetFileData.call(this, node);
            };
        }

        const origExpand = MEMFS.expandFileStorage;
        if (typeof origExpand === "function") {
            MEMFS.expandFileStorage = function (node, newCapacity) {
                ensureMaterialized(node);
                return origExpand.call(this, node, newCapacity);
            };
        }

        const origResize = MEMFS.resizeFileStorage;
        if (typeof origResize === "function") {
            MEMFS.resizeFileStorage = function (node, newSize) {
                if (newSize === 0) {
                    markNodeOverwritten(node);
                } else {
                    ensureMaterialized(node);
                }
                return origResize.call(this, node, newSize);
            };
        }

        function wrapLookup(origLookup) {
            return function (parent, name) {
                const hit = resolveChildNode(parent, name);
                if (hit) return hit;
                return origLookup.call(this, parent, name);
            };
        }

        function wrapGetattr(origGetattr) {
            return function (node) {
                const attr = origGetattr.call(this, node);
                if (node && node._diskRelPath && !node._materialized && typeof node._diskSize === "number") {
                    attr.size = node._diskSize;
                }
                return attr;
            };
        }

        function wrapSetattr(origSetattr) {
            return function (node, attr) {
                if (attr && attr.size !== undefined) {
                    if (attr.size === 0) {
                        markNodeOverwritten(node);
                    } else {
                        ensureMaterialized(node);
                    }
                }
                return origSetattr.call(this, node, attr);
            };
        }

        function wrapUnlink(origUnlink) {
            return function (parent, name) {
                const actual = resolveChildNode(parent, name);
                const targetName = actual ? actual.name : name;
                if (parent && parent._ciMap) {
                    delete parent._ciMap[targetName.toLowerCase()];
                }
                return origUnlink.call(this, parent, targetName);
            };
        }

        function wrapRead(origRead) {
            return function (stream, buffer, offset, length, position) {
                ensureMaterialized(stream.node);
                return origRead.call(this, stream, buffer, offset, length, position);
            };
        }

        function wrapWrite(origWrite) {
            return function (stream, buffer, offset, length, position, canOwn) {
                const node = stream.node;
                if (node && node._diskRelPath && !node._materialized) {
                    if ((position === 0 || position === undefined) && length >= (node._diskSize || 0)) {
                        markNodeOverwritten(node);
                    } else {
                        ensureMaterialized(node);
                    }
                }
                return origWrite.call(this, stream, buffer, offset, length, position, canOwn);
            };
        }

        function wrapMmap(origMmap) {
            return function (stream, length, position, prot, flags) {
                ensureMaterialized(stream.node);
                return origMmap.call(this, stream, length, position, prot, flags);
            };
        }

        MEMFS.node_ops.lookup = wrapLookup(MEMFS.node_ops.lookup);
        MEMFS.node_ops.getattr = wrapGetattr(MEMFS.node_ops.getattr);
        MEMFS.node_ops.setattr = wrapSetattr(MEMFS.node_ops.setattr);
        MEMFS.node_ops.unlink = wrapUnlink(MEMFS.node_ops.unlink);

        MEMFS.stream_ops.read = wrapRead(MEMFS.stream_ops.read);
        MEMFS.stream_ops.write = wrapWrite(MEMFS.stream_ops.write);
        MEMFS.stream_ops.mmap = wrapMmap(MEMFS.stream_ops.mmap);

        if (MEMFS.ops_table) {
            if (MEMFS.ops_table.dir && MEMFS.ops_table.dir.node) {
                MEMFS.ops_table.dir.node.lookup = MEMFS.node_ops.lookup;
                MEMFS.ops_table.dir.node.getattr = MEMFS.node_ops.getattr;
                MEMFS.ops_table.dir.node.setattr = MEMFS.node_ops.setattr;
                MEMFS.ops_table.dir.node.unlink = MEMFS.node_ops.unlink;
            }
            if (MEMFS.ops_table.file) {
                if (MEMFS.ops_table.file.node) {
                    MEMFS.ops_table.file.node.getattr = MEMFS.node_ops.getattr;
                    MEMFS.ops_table.file.node.setattr = MEMFS.node_ops.setattr;
                }
                if (MEMFS.ops_table.file.stream) {
                    MEMFS.ops_table.file.stream.read = MEMFS.stream_ops.read;
                    MEMFS.ops_table.file.stream.write = MEMFS.stream_ops.write;
                    MEMFS.ops_table.file.stream.mmap = MEMFS.stream_ops.mmap;
                }
            }
        }

        // Prevent IDBFS periodic background sync from materializing untouched local-disk worlds into IndexedDB
        if (typeof IDBFS !== "undefined" && typeof IDBFS.getLocalSet === "function") {
            const origGetLocalSet = IDBFS.getLocalSet;
            IDBFS.getLocalSet = function (mount, callback) {
                origGetLocalSet.call(this, mount, function (err, localSet) {
                    if (!err && localSet && localSet.entries) {
                        for (const fullPath of Object.keys(localSet.entries)) {
                            try {
                                const looked = FS.lookupPath(fullPath);
                                if (looked && looked.node && looked.node._diskRelPath && !looked.node._materialized) {
                                    delete localSet.entries[fullPath];
                                }
                            } catch (e) {
                                // ignore
                            }
                        }
                    }
                    callback(err, localSet);
                });
            };
        }
    }

    function ensureDirNode(dirParts) {
        let cur = FS.root;
        let curPath = "";
        for (let i = 0; i < dirParts.length; i++) {
            const part = dirParts[i];
            curPath += "/" + part;
            let child = resolveChildNode(cur, part);
            if (!child) {
                FS.mkdir(curPath);
                child = resolveChildNode(cur, part);
            } else if (child.mounted && child.mounted.root) {
                child = child.mounted.root;
            }
            if (!child) return null;
            cur = child;
        }
        return cur;
    }

    function registerEntryNode(relPath, size, mtimeMs, overwriteExisting, preloadedBytes) {
        const parts = relPath.split("/").filter(Boolean);
        if (!parts.length) return null;

        // Canonicalize top-level database -> Database when mounted
        if (parts[0].toLowerCase() === "database" && resolveChildNode(FS.root, "Database")) {
            parts[0] = "Database";
        } else if (parts[0].toLowerCase() === "worlds" && resolveChildNode(FS.root, "worlds")) {
            parts[0] = "worlds";
        } else if (parts[0].toLowerCase() === "apps" && resolveChildNode(FS.root, "apps")) {
            parts[0] = "apps";
        }

        const fileName = parts[parts.length - 1];
        const parent = ensureDirNode(parts.slice(0, -1));
        if (!parent) return null;

        let existing = resolveChildNode(parent, fileName);
        if (existing) {
            if (!overwriteExisting) return existing;
            try {
                delete parent.contents[existing.name];
            } catch (e) {
                // ignore
            }
        }

        const node = MEMFS.createNode(parent, fileName, 33206 /* S_IFREG | 0666 */, 0);
        registerCiChild(parent, fileName);
        if (mtimeMs) {
            node.mtime = mtimeMs;
            node.atime = mtimeMs;
            node.ctime = mtimeMs;
        }

        if (preloadedBytes) {
            node.contents = preloadedBytes;
            node.usedBytes = preloadedBytes.byteLength;
            node._diskSize = preloadedBytes.byteLength;
            node._diskRelPath = relPath;
            node._materialized = true;
        } else {
            node.contents = MEMFS.emptyFileContents || new Uint8Array(0);
            node.usedBytes = size;
            node._diskSize = size;
            node._diskRelPath = relPath;
            node._materialized = false;
        }
        return node;
    }

    async function fetchConcurrencyPool(items, concurrency, workerFn) {
        let index = 0;
        const runners = [];
        const limit = Math.max(1, Math.min(concurrency, items.length));
        for (let i = 0; i < limit; i++) {
            runners.push((async function () {
                while (index < items.length) {
                    const current = items[index++];
                    await workerFn(current);
                }
            })());
        }
        await Promise.all(runners);
    }

    async function initializeLocalDiskPreRun(Module) {
        patchMemfsOnce();

        const rawSearchRoot = getQueryParam("searchroot");
        const rawSearchPath = getQueryParam("searchpath");
        state.searchpath = rawSearchPath;

        const manifestUrl =
            state.fsOrigin +
            "/fs/manifest?root=" +
            encodeURIComponent(rawSearchRoot) +
            "&searchpath=" +
            encodeURIComponent(rawSearchPath);

        const t0 = performance.now();
        let manifest;
        try {
            const res = await fetch(manifestUrl, { cache: "no-store" });
            manifest = await res.json();
            if (!res.ok || !manifest || !manifest.ok) {
                console.warn("[local_disk_fs] Manifest unavailable:", manifest && manifest.error);
                return;
            }
        } catch (err) {
            console.warn("[local_disk_fs] Local /fs/manifest not reachable:", err);
            return;
        }

        state.resolvedRoot = manifest.root || rawSearchRoot;
        const entries = Array.isArray(manifest.entries) ? manifest.entries : [];
        state.stats.manifestEntries = entries.length;

        const bootFetchItems = [];
        const preloadedMap = new Map();

        for (let i = 0; i < entries.length; i++) {
            const entry = entries[i];
            const relPath = entry[0];
            const size = Number(entry[1]) || 0;
            if (shouldPrestageAtBoot(relPath, size)) {
                bootFetchItems.push(entry);
            }
        }

        let completedBytes = 0;
        let totalBootBytes = 0;
        for (let i = 0; i < bootFetchItems.length; i++) {
            totalBootBytes += Number(bootFetchItems[i][1]) || 0;
        }

        await fetchConcurrencyPool(bootFetchItems, 6, async function (entry) {
            const relPath = entry[0];
            try {
                const res = await fetch(buildFileUrl(relPath), { cache: "no-store" });
                if (!res.ok) throw new Error("HTTP " + res.status);
                const buf = await res.arrayBuffer();
                const bytes = new Uint8Array(buf);
                preloadedMap.set(relPath, bytes);
                completedBytes += bytes.byteLength;
                state.stats.prestagedFiles += 1;
                state.stats.prestagedBytes += bytes.byteLength;
                if (Module && typeof Module.setStatus === "function" && totalBootBytes > 0) {
                    const mbDone = (completedBytes / (1024 * 1024)).toFixed(1);
                    const mbTotal = (totalBootBytes / (1024 * 1024)).toFixed(1);
                    Module.setStatus(`Loading local disk (${mbDone}/${mbTotal})`);
                }
            } catch (err) {
                console.warn("[local_disk_fs] Prestage skipped for", relPath, err);
            }
        });

        for (let i = 0; i < entries.length; i++) {
            const entry = entries[i];
            const relPath = entry[0];
            const size = Number(entry[1]) || 0;
            const mtimeMs = Number(entry[2]) || Date.now();
            const slash = relPath.indexOf("/");
            const topDir = (slash >= 0 ? relPath.substring(0, slash) : "").toLowerCase();

            // Defer /apps, /worlds, /Database until main.cpp mounts IDBFS on them
            if (DEFERRED_MOUNT_TOP_DIRS.has(topDir)) {
                state.deferredEntries.push([relPath, size, mtimeMs, preloadedMap.get(relPath) || null]);
                continue;
            }

            registerEntryNode(relPath, size, mtimeMs, true, preloadedMap.get(relPath) || null);
        }

        const elapsedMs = Math.round(performance.now() - t0);
        console.log(
            `[local_disk_fs] Mounted ${entries.length} entries from ${state.resolvedRoot} ` +
            `(prestaged ${state.stats.prestagedFiles} files / ${(state.stats.prestagedBytes / (1024 * 1024)).toFixed(1)} MB in ${elapsedMs} ms)`
        );
    }

    function mergeDeferredMountDirs() {
        if (!state.deferredEntries.length) return;
        const list = state.deferredEntries.splice(0, state.deferredEntries.length);
        let merged = 0;
        for (let i = 0; i < list.length; i++) {
            const [relPath, size, mtimeMs, preloadedBytes] = list[i];
            // Do not overwrite user-saved files already restored from IDBFS
            const node = registerEntryNode(relPath, size, mtimeMs, false, preloadedBytes);
            if (node) merged += 1;
        }
        console.log(`[local_disk_fs] Merged ${merged} local disk entries into IDBFS (/worlds, /Database, /apps)`);
    }

    function install(Module) {
        if (state.installed) return;
        state.installed = true;
        Module.preRun = Module.preRun || [];
        if (typeof Module.preRun === "function") {
            Module.preRun = [Module.preRun];
        }
        Module.preRun.push(function () {
            const addDep = typeof addRunDependency === "function"
                ? addRunDependency
                : (Module.addRunDependency && Module.addRunDependency.bind(Module));
            const removeDep = typeof removeRunDependency === "function"
                ? removeRunDependency
                : (Module.removeRunDependency && Module.removeRunDependency.bind(Module));

            if (addDep) addDep("local_disk_fs");
            initializeLocalDiskPreRun(Module).finally(function () {
                if (removeDep) removeDep("local_disk_fs");
            });
        });
    }

    window.LocalDiskFS = {
        Install: install,
        MergeDeferredMountDirs: mergeDeferredMountDirs,
        GetState: function () { return state; },
    };
})();
