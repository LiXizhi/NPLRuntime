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
        "fonts/tahoma.ttf",
        "fonts/tahomabd.ttf",
        "fonts/opensans-regular.ttf",
        "fonts/opensans-semibold.ttf",
        "npl_packages/paracraftbuildinmod.zip",
        "database/characters.db",
        "config/characters.db",
    ]);

    const BUNDLED_WEB_FONTS = [
        ["fonts/OpenSans-Regular.ttf", "fonts/OpenSans-Regular.ttf"],
        ["fonts/OpenSans-Semibold.ttf", "fonts/OpenSans-Semibold.ttf"],
    ];

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
        if (lower.startsWith("fonts/") && lower.indexOf("/", 6) < 0 && size <= 4 * 1024 * 1024) return true;
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
        const url = buildFileUrl(relPath) + "&enc=b64";
        const xhr = new XMLHttpRequest();
        xhr.open("GET", url, false);
        xhr.send(null);
        if (xhr.status !== 200) {
            throw new Error("HTTP " + xhr.status + " for " + relPath);
        }
        const b64 = (xhr.responseText || "").trim();
        if (!b64) {
            return new Uint8Array(0);
        }
        const bin = atob(b64);
        const bytes = new Uint8Array(bin.length);
        for (let i = 0; i < bin.length; i++) {
            bytes[i] = bin.charCodeAt(i);
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
            const isBrokenEmptyStub = !existing._diskRelPath && (existing.usedBytes === 0) && (size > 0);
            if (!overwriteExisting && !isBrokenEmptyStub) return existing;
            try {
                delete parent.contents[existing.name];
            } catch (e) {
                // ignore
            }
        }

        // Keep the writable node operations installed by the Web /temp mount.
        const create = parent.mount.mountpoint === "/temp" ? parent.node_ops.mknod : MEMFS.createNode;
        const node = create.call(MEMFS, parent, fileName, 33206 /* S_IFREG | 0666 */, 0);
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

    function shouldBatchPrestageScript(relPath, size) {
        if (size <= 0 || size > 128 * 1024) return false;
        const lower = relPath.toLowerCase();
        if (!lower.startsWith("script/") && !lower.startsWith("mod/")) return false;
        return lower.endsWith(".lua") || lower.endsWith(".npl") || lower.endsWith(".xml");
    }

    async function fetchBatchPack(relPaths, preloadedMap, onChunkBytes) {
        if (!relPaths.length) return true;
        const res = await fetch(state.fsOrigin + "/fs/batch", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ root: state.resolvedRoot, paths: relPaths }),
            cache: "no-store",
        });
        if (!res.ok) return false;
        const buf = await res.arrayBuffer();
        if (buf.byteLength < 8) return false;
        const view = new DataView(buf);
        // "NPB1" = 0x3142504e little-endian
        if (view.getUint32(0, true) !== 0x3142504e) return false;
        const count = view.getUint32(4, true);
        const decoder = new TextDecoder("utf-8");
        let offset = 8;
        let unpackedBytes = 0;
        for (let i = 0; i < count; i++) {
            if (offset + 2 > buf.byteLength) break;
            const pathLen = view.getUint16(offset, true);
            offset += 2;
            if (offset + pathLen + 4 > buf.byteLength) break;
            const relPath = decoder.decode(new Uint8Array(buf, offset, pathLen));
            offset += pathLen;
            const dataLen = view.getUint32(offset, true);
            offset += 4;
            if (offset + dataLen > buf.byteLength) break;
            const fileBytes = new Uint8Array(buf, offset, dataLen);
            offset += dataLen;
            preloadedMap.set(relPath, fileBytes);
            state.stats.prestagedFiles += 1;
            state.stats.prestagedBytes += dataLen;
            unpackedBytes += dataLen;
        }
        if (onChunkBytes && unpackedBytes > 0) onChunkBytes(unpackedBytes);
        return true;
    }

    async function ensureBundledWebFonts() {
        const baseOrigin = (typeof window !== "undefined" && window.WEBPARACRAFT_ORIGIN)
            ? window.WEBPARACRAFT_ORIGIN
            : (state.fsOrigin + "/");
        await Promise.all(BUNDLED_WEB_FONTS.map(async function (pair) {
            const memRelPath = pair[0];
            const assetSubPath = pair[1];
            try {
                const existing = FS.analyzePath("/" + memRelPath);
                if (existing && existing.exists) return;
            } catch (e) {
                // ignore
            }
            try {
                const url = new URL(assetSubPath, baseOrigin).href;
                const res = await fetch(url, { cache: "force-cache" });
                if (!res.ok) return;
                const bytes = new Uint8Array(await res.arrayBuffer());
                if (bytes.byteLength > 0) {
                    registerEntryNode(memRelPath, bytes.byteLength, Date.now(), false, bytes);
                }
            } catch (e) {
                // ignore optional bundled web font fetch errors
            }
        }));
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
                await ensureBundledWebFonts();
                return;
            }
        } catch (err) {
            console.warn("[local_disk_fs] Local /fs/manifest not reachable:", err);
            await ensureBundledWebFonts();
            return;
        }

        state.resolvedRoot = manifest.root || rawSearchRoot;
        const entries = Array.isArray(manifest.entries) ? manifest.entries : [];
        state.stats.manifestEntries = entries.length;

        const bootLargeItems = [];
        const bootBatchEntries = [];
        const preloadedMap = new Map();
        let scriptBatchBytes = 0;
        const maxScriptBatchBytes = 24 * 1024 * 1024;
        const maxScriptBatchFiles = 2000;
        let scriptBatchCount = 0;

        for (let i = 0; i < entries.length; i++) {
            const entry = entries[i];
            const relPath = entry[0];
            const size = Number(entry[1]) || 0;
            if (shouldPrestageAtBoot(relPath, size)) {
                if (size > 0 && size <= 512 * 1024) {
                    bootBatchEntries.push(entry);
                } else {
                    bootLargeItems.push(entry);
                }
            } else if (
                scriptBatchCount < maxScriptBatchFiles &&
                scriptBatchBytes + size <= maxScriptBatchBytes &&
                shouldBatchPrestageScript(relPath, size)
            ) {
                bootBatchEntries.push(entry);
                scriptBatchBytes += size;
                scriptBatchCount += 1;
            }
        }

        let completedBytes = 0;
        let totalBootBytes = 0;
        for (let i = 0; i < bootLargeItems.length; i++) {
            totalBootBytes += Number(bootLargeItems[i][1]) || 0;
        }
        for (let i = 0; i < bootBatchEntries.length; i++) {
            totalBootBytes += Number(bootBatchEntries[i][1]) || 0;
        }

        const updateBootStatus = function () {
            if (Module && typeof Module.setStatus === "function" && totalBootBytes > 0) {
                const mbDone = (completedBytes / (1024 * 1024)).toFixed(1);
                const mbTotal = (totalBootBytes / (1024 * 1024)).toFixed(1);
                Module.setStatus(`Loading local disk (${mbDone}/${mbTotal})`);
            }
        };

        const batchChunks = [];
        const batchChunkSize = 500;
        for (let i = 0; i < bootBatchEntries.length; i += batchChunkSize) {
            batchChunks.push(bootBatchEntries.slice(i, i + batchChunkSize));
        }

        let batchFallbackEntries = [];
        await Promise.all([
            fetchConcurrencyPool(bootLargeItems, 6, async function (entry) {
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
                    updateBootStatus();
                } catch (err) {
                    console.warn("[local_disk_fs] Prestage skipped for", relPath, err);
                }
            }),
            fetchConcurrencyPool(batchChunks, 2, async function (chunk) {
                const paths = chunk.map(function (e) { return e[0]; });
                try {
                    const ok = await fetchBatchPack(paths, preloadedMap, function (added) {
                        completedBytes += added;
                        updateBootStatus();
                    });
                    if (!ok) {
                        batchFallbackEntries = batchFallbackEntries.concat(chunk.filter(function (e) {
                            return shouldPrestageAtBoot(e[0], Number(e[1]) || 0);
                        }));
                    }
                } catch (err) {
                    batchFallbackEntries = batchFallbackEntries.concat(chunk.filter(function (e) {
                        return shouldPrestageAtBoot(e[0], Number(e[1]) || 0);
                    }));
                }
            }),
        ]);

        if (batchFallbackEntries.length > 0) {
            await fetchConcurrencyPool(batchFallbackEntries, 6, async function (entry) {
                const relPath = entry[0];
                if (preloadedMap.has(relPath)) return;
                try {
                    const res = await fetch(buildFileUrl(relPath), { cache: "no-store" });
                    if (!res.ok) throw new Error("HTTP " + res.status);
                    const bytes = new Uint8Array(await res.arrayBuffer());
                    preloadedMap.set(relPath, bytes);
                    completedBytes += bytes.byteLength;
                    state.stats.prestagedFiles += 1;
                    state.stats.prestagedBytes += bytes.byteLength;
                    updateBootStatus();
                } catch (err) {
                    console.warn("[local_disk_fs] Fallback prestage skipped for", relPath, err);
                }
            });
        }

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

        await ensureBundledWebFonts();

        const elapsedMs = Math.round(performance.now() - t0);
        console.log(
            `[local_disk_fs] Mounted ${entries.length} entries from ${state.resolvedRoot} ` +
            `(prestaged ${state.stats.prestagedFiles} files / ${(state.stats.prestagedBytes / (1024 * 1024)).toFixed(1)} MB in ${elapsedMs} ms)`
        );
    }

    function tryUnlinkIfExists(fullPath) {
        try {
            const info = FS.analyzePath(fullPath);
            if (info && info.exists) {
                FS.unlink(fullPath);
                return true;
            }
        } catch (e) {
            // ignore
        }
        return false;
    }

    function cleanupContaminatedIdbfs() {
        const forceReset = getQueryParam("resetidb") === "1" || getQueryParam("resetidb") === "true";
        let hasDesktopContamination = false;
        try {
            if (
                FS.analyzePath("/Database/localserver.ver").exists ||
                FS.analyzePath("/Database/npl/project.db-wal").exists ||
                FS.analyzePath("/Database/npl/project.db-shm").exists
            ) {
                hasDesktopContamination = true;
            }
        } catch (e) {
            // ignore
        }

        if (forceReset || hasDesktopContamination) {
            const staleDbPaths = [
                "/Database/localserver.db",
                "/Database/localserver.ver",
                "/Database/Account.db",
                "/Database/account.db",
                "/Database/userdata.db",
                "/Database/creator_profile.db",
                "/Database/npl/project.db",
                "/Database/npl/project.db-shm",
                "/Database/npl/project.db-wal",
            ];
            let removed = 0;
            for (let i = 0; i < staleDbPaths.length; i++) {
                if (tryUnlinkIfExists(staleDbPaths[i])) removed += 1;
            }
            if (removed > 0) {
                console.log(`[local_disk_fs] Cleaned ${removed} stale desktop cache DB file(s) from IDBFS /Database`);
            }
        }

        // Remove any 0-byte corrupted .zip/.pkg stubs in /worlds/DesignHouse/userworlds
        try {
            const uwPath = "/worlds/DesignHouse/userworlds";
            if (FS.analyzePath(uwPath).exists) {
                const names = FS.readdir(uwPath);
                for (let i = 0; i < names.length; i++) {
                    const name = names[i];
                    if (name === "." || name === "..") continue;
                    const lower = name.toLowerCase();
                    if (!lower.endsWith(".zip") && !lower.endsWith(".pkg")) continue;
                    const full = uwPath + "/" + name;
                    try {
                        const looked = FS.lookupPath(full);
                        const node = looked && looked.node;
                        if (node && !node._diskRelPath && node.usedBytes === 0) {
                            FS.unlink(full);
                            console.log("[local_disk_fs] Removed 0-byte world archive stub:", full);
                        }
                    } catch (e) {
                        // ignore
                    }
                }
            }
        } catch (e) {
            // ignore
        }
    }

    function mergeDeferredMountDirs() {
        if (typeof FS !== "undefined" && FS.root && FS.root.contents) {
            for (const key of Object.keys(FS.root.contents)) {
                registerCiChild(FS.root, key);
            }
        }
        cleanupContaminatedIdbfs();
        if (!state.deferredEntries.length) return;
        const list = state.deferredEntries.splice(0, state.deferredEntries.length);
        let merged = 0;
        for (let i = 0; i < list.length; i++) {
            const [relPath, size, mtimeMs, preloadedBytes] = list[i];
            // Do not overwrite valid user-saved files already restored from IDBFS
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
