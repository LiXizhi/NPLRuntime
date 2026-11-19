// Opt-in diagnostic. Capture one rendered frame of the local WebGL client.
// The capture deliberately reads GL state and must not be used for timings.
document.addEventListener("DOMContentLoaded", () => {
    const panel = document.createElement("div");
    panel.style.cssText = "position:fixed;bottom:8px;right:8px;z-index:2147483647;background:white;padding:4px;font:14px sans-serif";
    const button = document.createElement("button");
    button.textContent = "Capture scene draws";
    const download = document.createElement("a");
    const instancing = document.createElement("button");
    instancing.textContent = "Toggle scene instancing";
    instancing.onclick = () => {
        const bridge = window.EmscriptenInstance;
        if (!bridge || !bridge.ExecuteLuaCode) return;
        const toggle = 'local scene = ParaScene.GetAttributeObject(); local old = scene:GetField("UseInstancing", false); ' +
            'scene:SetField("UseInstancing", not old); return tostring(scene:GetField("UseInstancing", old))';
        bridge.ExecuteLuaCode(toggle, {}, reply => {
            instancing.textContent = reply.success ? "Scene instancing: " + reply.result : "Instancing switch unavailable";
        });
    };
    const compare = document.createElement("button");
    compare.textContent = "Compare scenery";
    function clearDownload() {
        if (download.href) URL.revokeObjectURL(download.href);
        download.removeAttribute("href");
        download.textContent = "";
    }
    compare.onclick = async () => {
        clearDownload();
        compare.disabled = button.disabled = instancing.disabled = true;
        const gl = document.querySelector("#canvas").getContext("webgl2");
        const lua = code => new Promise((resolve, reject) => {
            window.EmscriptenInstance.ExecuteLuaCode(code, {}, reply =>
                reply.success ? resolve(reply.result) : reject(new Error("Scene switch failed")));
        });
        const attrs = 'local s=ParaScene.GetAttributeObject(); local t=ParaTerrain.GetAttributeObject(); ';
        const states = 'return tostring(s:GetField("UseInstancing",false))..","..tostring(t:GetField("UseExtendedTerrainLayers",nil))';
        async function setModes(mesh, terrain) {
            return lua(attrs + 's:SetField("UseInstancing",' + mesh + '); t:SetField("UseExtendedTerrainLayers",' + terrain + '); ' + states);
        }
        let initial;
        try {
            initial = String(await lua(attrs + states)).split(",");
            if (!initial.every(x => x === "true" || x === "false"))
                throw new Error("This engine does not expose both switches");
            const captures = [];
            for (const [mesh, terrain] of [[false,false], [false,true], [true,true], [false,false]]) {
                const flags = String(await setModes(mesh, terrain));
                if (flags !== [mesh, terrain].join(",")) throw new Error("Scene switches did not take effect");
                compare.textContent = "Comparing " + (captures.length + 1) + "/4";
                const frame = await captureSceneDraws(gl, true);
                if (!frame.image) throw new Error("Load the outdoor scene first");
                captures.push({mesh, terrain, ...frame});
            }
            download.href = URL.createObjectURL(new Blob([JSON.stringify(captures)], {type: "application/json"}));
            download.download = "haqi-scenery-comparison-" + Date.now() + ".json";
            download.textContent = "Save scenery comparison";
        } catch (error) {
            download.textContent = error.message;
            download.removeAttribute("href");
        } finally {
            try {
                if (initial && initial.every(x => x === "true" || x === "false")) await setModes(...initial);
            } finally {
                compare.textContent = "Compare scenery";
                compare.disabled = button.disabled = instancing.disabled = false;
            }
        }
    };
    panel.append(button, download, instancing, compare);
    document.body.append(panel);
    button.onclick = async () => {
        clearDownload();
        compare.disabled = button.disabled = instancing.disabled = true;
        try {
            const gl = document.querySelector("#canvas").getContext("webgl2");
            const result = await captureSceneDraws(gl);
            download.href = URL.createObjectURL(new Blob([JSON.stringify(result)], {type: "application/json"}));
            download.download = "haqi-scene-draws-" + Date.now() + ".json";
            download.textContent = "Save " + result.draws.length + " draws";
        } catch (error) {
            download.removeAttribute("href");
            download.textContent = "Capture failed: " + error.message;
        } finally {
            compare.disabled = button.disabled = instancing.disabled = false;
        }
    };
});

async function captureSceneDraws(gl, isolateScenery = false) {
    let pixels, sceneryPending = false;
    const draws = [], programs = {}, stacks = [];
    const ids = new WeakMap(), transforms = new Map(), stackIndex = new Map(), locations = new Map();
    const materials = new Map(), materialLocations = new Map();
    let nextId = 1, captureError;
    function id(object) {
        if (!object) return 0;
        if (!ids.has(object)) ids.set(object, nextId++);
        return ids.get(object);
    }
    function inspectProgram(program) {
        const uniforms = [], values = [];
        for (let i = 0; i < gl.getProgramParameter(program, gl.ACTIVE_UNIFORMS); ++i) {
            const u = gl.getActiveUniform(program, i);
            uniforms.push(u.name);
            if (/^mWorld(View(Proj)?)?$/i.test(u.name)) continue;
            for (let j = 0; j < u.size; ++j)
                values.push(gl.getUniformLocation(program, u.name.replace("[0]", "[" + j + "]")));
        }
        programs[id(program)] = uniforms;
        const wvp = uniforms.find(name => /^mWorldViewProj$/i.test(name));
        locations.set(program, wvp ? gl.getUniformLocation(program, wvp) : null);
        // Check material equivalence only for the two repeated scenery shader families.
        if (uniforms.includes("g_bRGBOnlyTexturAnim") || uniforms.includes("g_parameter0"))
            materialLocations.set(program, values);
    }
    function record(method, args) {
        const program = gl.getParameter(gl.CURRENT_PROGRAM);
        if (!programs[id(program)]) inspectProgram(program);
        const loc = locations.get(program);
        let transform = null;
        if (loc) {
            const key = gl.getUniform(program, loc).join(",");
            if (!transforms.has(key)) transforms.set(key, transforms.size + 1);
            transform = transforms.get(key);
        }
        // Keep only engine frames. This excludes URLs and all gameplay data.
        const trace = Array.from(new Error().stack.matchAll(/wasm-function\[(\d+)\]/g), m => +m[1]);
        const key = trace.join(",");
        if (!stackIndex.has(key)) {
            stackIndex.set(key, stacks.length);
            stacks.push(trace);
        }
        let material = null;
        if (materialLocations.has(program)) {
            const key = materialLocations.get(program).map(l => String(gl.getUniform(program, l))).join("|");
            if (!materials.has(key)) materials.set(key, materials.size + 1);
            material = materials.get(key);
        }
        const active = gl.getParameter(gl.ACTIVE_TEXTURE);
        const textures = [];
        try {
            for (let unit = 0; unit < 2; ++unit) {
                gl.activeTexture(gl.TEXTURE0 + unit);
                textures.push(id(gl.getParameter(gl.TEXTURE_BINDING_2D)));
            }
        } finally {
            gl.activeTexture(active);
        }
        const indexed = method.startsWith("drawElements");
        draws.push({stack: stackIndex.get(key), shader: id(program), method, mode: args[0],
            count: args[indexed ? 1 : 2], offset: args[indexed ? 3 : 1],
            indexType: indexed ? args[2] : null, instances: method.endsWith("Instanced") ? args.at(-1) : 1,
            target: id(gl.getParameter(gl.DRAW_FRAMEBUFFER_BINDING)), textures, transform, material,
            vertices: id(gl.getVertexAttrib(0, gl.VERTEX_ATTRIB_ARRAY_BUFFER_BINDING)),
            indices: indexed ? id(gl.getParameter(gl.ELEMENT_ARRAY_BUFFER_BINDING)) : 0,
            blend: gl.isEnabled(gl.BLEND), depthWrite: gl.getParameter(gl.DEPTH_WRITEMASK),
            cull: gl.isEnabled(gl.CULL_FACE) ? gl.getParameter(gl.CULL_FACE_MODE) : 0,
            winding: gl.getParameter(gl.FRONT_FACE), depth: gl.isEnabled(gl.DEPTH_TEST) ? gl.getParameter(gl.DEPTH_FUNC) : 0,
            blendSrc: gl.getParameter(gl.BLEND_SRC_RGB), blendDst: gl.getParameter(gl.BLEND_DST_RGB)});
    }
    const originals = new Map();
    const oldLimit = Error.stackTraceLimit;
    const frame = () => new Promise(requestAnimationFrame);
    await frame();
    try {
        Error.stackTraceLimit = 45;
        for (const name of ["drawArrays", "drawElements", "drawArraysInstanced", "drawElementsInstanced"]) {
            const original = gl[name];
            originals.set(name, Object.getOwnPropertyDescriptor(gl, name));
            gl[name] = function (...args) {
                if (!captureError) {
                    try {
                        record(name, args);
                        if (isolateScenery) {
                            const draw = draws[draws.length - 1];
                            const uniforms = programs[draw.shader];
                            const terrain = uniforms.includes("g_bIsBaseEnabled") || uniforms.includes("g_vertexOffset");
                            const scenery = name.startsWith("drawElements") && uniforms.includes("g_bRGBOnlyTexturAnim");
                            if (uniforms.includes("g_layerCount")) {
                                const program = gl.getParameter(gl.CURRENT_PROGRAM);
                                draw.layers = gl.getUniform(program, gl.getUniformLocation(program, "g_layerCount"));
                            }
                            if (!draw.target && (terrain || scenery)) {
                                sceneryPending = true;
                            } else if (!draw.target) {
                                if (sceneryPending) {
                                    pixels ??= new Uint8Array(gl.drawingBufferWidth * gl.drawingBufferHeight * 4);
                                    gl.readPixels(0, 0, gl.drawingBufferWidth, gl.drawingBufferHeight, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
                                    sceneryPending = false;
                                }
                                // Suppress actors, foliage and HUD only in this diagnostic frame.
                                return;
                            }
                        }
                    }
                    catch (error) { captureError = error; }
                }
                return original.apply(this, args);
            };
        }
        for (let i = 0; i < 120 && !draws.length && !captureError; ++i) await frame();
    } finally {
        for (const [name, descriptor] of originals) {
            if (descriptor) Object.defineProperty(gl, name, descriptor);
            else delete gl[name];
        }
        Error.stackTraceLimit = oldLimit;
    }
    if (captureError) throw captureError;
    let image;
    if (pixels) {
        const canvas = document.createElement("canvas");
        canvas.width = gl.drawingBufferWidth;
        canvas.height = gl.drawingBufferHeight;
        const context = canvas.getContext("2d");
        const data = context.createImageData(canvas.width, canvas.height);
        const rowBytes = canvas.width * 4;
        for (let y = 0; y < canvas.height; ++y) {
            const start = (canvas.height - y - 1) * rowBytes;
            data.data.set(pixels.subarray(start, start + rowBytes), y * rowBytes);
        }
        context.putImageData(data, 0, 0);
        image = canvas.toDataURL("image/png");
    }
    return {image, width: gl.drawingBufferWidth, height: gl.drawingBufferHeight,
        textureUnits: gl.getParameter(gl.MAX_TEXTURE_IMAGE_UNITS), programs, stacks, draws};
}
