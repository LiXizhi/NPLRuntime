// Local WebParacraft asset caching and embedded NPL WebServer bridge.

const NPL_WEBSERVER_ROUTE = "/__npl_webserver__";
let nplWebserverClientId = "";

self.addEventListener("install", event => {
    self.skipWaiting();
});

self.addEventListener("activate", event => {
    event.waitUntil(self.clients.claim());
});

self.addEventListener("message", event => {
    const data = event.data || {};
    if (data.type === "npl-webserver-register" && event.source && event.source.id) {
        nplWebserverClientId = event.source.id;
    }
});

function isDevHost() {
    const h = self.location.hostname;
    return h === "localhost" || h === "127.0.0.1";
}

function getWorkerDirectory() {
    return self.location.pathname.substring(0, self.location.pathname.lastIndexOf("/") + 1);
}

function isScopedWorker() {
    return getWorkerDirectory() !== "/";
}

function parseWebserverRoute(pathname) {
    const routeIndex = pathname.indexOf(NPL_WEBSERVER_ROUTE);
    if (routeIndex < 0) return null;
    const rest = pathname.substring(routeIndex + NPL_WEBSERVER_ROUTE.length);
    if (rest && !rest.startsWith("/")) return null;
    return {
        routeRoot: pathname.substring(0, routeIndex) + NPL_WEBSERVER_ROUTE,
        path: rest || "/",
    };
}

const WEBPARACRAFT_ROOT_PREFIXES = [
    "/auto_generate_files/",
    "/camera/",
    "/fs/",
    "/keepwork/",
    "/libs/",
    "/src/",
    "/webpage/",
];

const WEBPARACRAFT_ROOT_FILES = new Set([
    "/about_blank.html",
    "/fs",
    "/index.html",
    "/manifest.json",
    "/NPL.js",
    "/ParaCraftMutilThread.html",
    "/ParaCraftSingleThread.html",
    "/ServiceWorker",
    "/ServiceWorker.js",
    "/version.txt",
]);

function isWebparacraftPath(pathname) {
    if (pathname === "/ServiceWorker.js" || pathname === "/ServiceWorker") return true;
    if (pathname === "/fs" || pathname.startsWith("/fs/")) return true;
    if (isDevHost()) return pathname.startsWith("/npl/webparacraft/");
    if (pathname === "/" || /^\/pid\/\d+$/.test(pathname)) return true;
    if (WEBPARACRAFT_ROOT_FILES.has(pathname)) return true;
    return WEBPARACRAFT_ROOT_PREFIXES.some(prefix => pathname.startsWith(prefix));
}

function routeFromOriginRoot(pathname) {
    if (isScopedWorker() && pathname.startsWith(getWorkerDirectory())) return null;
    if (isWebparacraftPath(pathname)) return null;
    return { routeRoot: "", path: pathname };
}

function rewriteWikiHtml(text, routeRoot) {
    return text.replace(/\b(href|src|action)=(['"])\/(?!\/)/gi, "$1=$2" + routeRoot + "/");
}

function arrayBufferToBase64(buffer) {
    const bytes = new Uint8Array(buffer);
    let binary = "";
    for (let offset = 0; offset < bytes.length; offset += 0x8000) {
        binary += String.fromCharCode.apply(null, bytes.subarray(offset, offset + 0x8000));
    }
    return btoa(binary);
}

function base64ToArrayBuffer(base64) {
    const binary = atob(base64 || "");
    const bytes = new Uint8Array(binary.length);
    for (let i = 0; i < binary.length; i++) bytes[i] = binary.charCodeAt(i);
    return bytes.buffer;
}

function requestNplWebserverFromTarget(target, request) {
    return new Promise((resolve, reject) => {
        const timer = setTimeout(() => reject(new Error("NPL WebServer request timeout")), 20000);
        const channel = new MessageChannel();
        channel.port1.onmessage = event => {
            clearTimeout(timer);
            const data = event.data || {};
            if (data.error) reject(new Error(String(data.error)));
            else resolve(data.response || {});
        };
        target.postMessage({ type: "npl-webserver-request", request }, [channel.port2]);
    });
}

async function requestNplWebserver(request) {
    if (nplWebserverClientId) {
        const owner = await self.clients.get(nplWebserverClientId);
        if (owner) return requestNplWebserverFromTarget(owner, request);
        nplWebserverClientId = "";
    }
    const clients = await self.clients.matchAll({ type: "window", includeUncontrolled: true });
    if (!clients.length) throw new Error("NPL WebServer page is not available");
    return new Promise((resolve, reject) => {
        let pending = clients.length;
        let lastError = null;
        for (const client of clients) {
            requestNplWebserverFromTarget(client, request).then(resolve).catch(error => {
                lastError = error;
                pending--;
                if (!pending) reject(lastError);
            });
        }
    });
}

async function proxyNplWebserver(event, route) {
    const source = new URL(event.request.url);
    const headers = {};
    for (const [name, value] of event.request.headers.entries()) {
        if (!name.startsWith("sec-") && name !== "host" && name !== "content-length") headers[name] = value;
    }
    let bodyBase64 = "";
    if (event.request.method !== "GET" && event.request.method !== "HEAD") {
        bodyBase64 = arrayBufferToBase64(await event.request.arrayBuffer());
    }
    const captured = await requestNplWebserver({
        method: event.request.method,
        path: route.path + source.search,
        headers: headers,
        bodyBase64: bodyBase64,
    });
    const status = Number(captured.status) || (captured.ok === false ? 502 : 200);
    const responseHeaders = new Headers(captured.headers || {});
    responseHeaders.delete("content-length");
    responseHeaders.delete("content-encoding");
    responseHeaders.delete("set-cookie");
    responseHeaders.set("Cross-Origin-Embedder-Policy", "require-corp");
    const contentType = responseHeaders.get("content-type") || "";
    const body = base64ToArrayBuffer(captured.bodyBase64 || "");
    const responseBody = status === 204 || status === 205 || status === 304 ? null : body;
    if (contentType.includes("text/html")) {
        const html = rewriteWikiHtml(new TextDecoder().decode(body), route.routeRoot);
        return new Response(html, { status: status, headers: responseHeaders });
    }
    return new Response(responseBody, { status: status, headers: responseHeaders });
}

// Large WASM/package responses can exceed the browser HTTP cache's per-entry limit.
async function serveLargeFile(event, key, revision) {
    let cache, cached;
    try {
        cache = await caches.open("WebParacraft-local-large-v1");
        cached = await cache.match(key);
    } catch (error) {
        return fetch(event.request);
    }
    const headers = new Headers(event.request.headers);
    if (cached) {
        const etag = cached.headers.get("ETag");
        if (revision && etag === '"' + revision + '"') return cached;
        headers.set("If-None-Match", etag);
    }
    const response = await fetch(event.request, { headers, cache: "no-store" });
    if (response.status === 304 && cached) return cached;
    if (response.status === 200 && response.headers.has("ETag")) {
        try {
            if (cached) await cache.delete(key);
            await cache.put(key, response.clone());
        } catch (error) {
            console.warn("WebParacraft could not retain", key.pathname, error.name);
        }
    }
    return response;
}

self.addEventListener("fetch", event => {
    const requestUrl = new URL(event.request.url);
    if (requestUrl.origin !== self.location.origin) return;

    if (event.request.method === "GET" && !event.request.headers.has("Range")) {
        const revision = requestUrl.searchParams.get("rev");
        const size = revision && parseInt(revision.split("-").pop(), 16);
        const largeDiskFile = requestUrl.pathname === "/fs/file" && size >= 8 * 1024 * 1024;
        if (largeDiskFile || /\/auto_generate_files\/ParaCraft(?:SingleThread)?\.wasm$/.test(requestUrl.pathname)) {
            const key = new URL(requestUrl);
            key.searchParams.delete("rev");
            event.respondWith(serveLargeFile(event, key, largeDiskFile && revision));
            return;
        }
    }

    if (requestUrl.pathname.endsWith("/manifest.json")) {
        const baseUrl = getWorkerDirectory();
        const pid = requestUrl.searchParams.get("pid") || "";
        const pwaName = "WebParaCraft" + pid;
        event.respondWith(new Response(JSON.stringify({
            id: pwaName,
            short_name: pwaName,
            name: pwaName,
            start_url: baseUrl + "index.html" + requestUrl.search,
            scope: baseUrl,
            display: "standalone",
        }), {
            status: 200,
            headers: {
                "Content-Type": "application/manifest+json; charset=utf-8",
                "Cross-Origin-Embedder-Policy": "require-corp",
            },
        }));
        return;
    }

    const webserverRoute = parseWebserverRoute(requestUrl.pathname) || routeFromOriginRoot(requestUrl.pathname);
    if (!webserverRoute) {
        return;
    }

    event.respondWith((async () => {
        try {
            return await proxyNplWebserver(event, webserverRoute);
        } catch (error) {
            return new Response(JSON.stringify({ ok: false, error: String(error) }), {
                status: 502,
                headers: {
                    "Content-Type": "application/json; charset=utf-8",
                    "Cross-Origin-Embedder-Policy": "require-corp",
                },
            });
        }
    })());
});
