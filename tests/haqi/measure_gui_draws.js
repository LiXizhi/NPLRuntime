// Paste into Chrome DevTools after a Haqi WebGL scene has warmed up.
// Resolves after 120 rendered frames and restores the original WebGL methods.
(async () => {
    const live = document.currentScript?.hasAttribute("data-live");
    const bench = live && new URLSearchParams(location.search).get("bench") === "1";
    let overlay, gl;
    if (live) {
        const context = new Promise(resolve => {
            const original = HTMLCanvasElement.prototype.getContext;
            HTMLCanvasElement.prototype.getContext = function (...args) {
                const value = original.apply(this, args);
                if (this.id === "canvas" && value && args[0] === "webgl2") {
                    HTMLCanvasElement.prototype.getContext = original;
                    resolve(value);
                }
                return value;
            };
        });
        document.addEventListener("DOMContentLoaded", () => {
            overlay = document.createElement("output");
            overlay.id = "haqi-draw-stats";
            overlay.style.cssText = "position:fixed;bottom:8px;left:8px;z-index:2147483647;background:#15191eee;color:white;padding:10px 14px;border-radius:5px;font:14px/1.5 monospace;white-space:pre;pointer-events:none";
            overlay.textContent = "Waiting for WebGL frames…";
            document.body.append(overlay);
        });
        gl = await context;
    } else {
        gl = document.querySelector("#canvas").getContext("webgl2");
    }
    if (!gl) throw new Error("A running WebGL 2 scene is required");
    const programs = new WeakMap();
    function guiKind(program) {
        if (!program) return 0;
        if (!programs.has(program)) {
            const names = [];
            for (let i = 0; i < gl.getProgramParameter(program, gl.ACTIVE_UNIFORMS); ++i)
                names.push(gl.getActiveUniform(program, i).name);
            const standard = names.length === 3 && ["mWorldViewProj", "k_bBoolean0", "tex0Sampler"].every(name => names.includes(name));
            let multi = names.length === 5 && ["mWorldViewProj", "tex0Sampler", "tex1Sampler", "tex2Sampler", "tex3Sampler"].every(name => names.includes(name));
            if (multi) {
                const attributes = Array.from({length: gl.getProgramParameter(program, gl.ACTIVE_ATTRIBUTES)}, (_, i) => gl.getActiveAttrib(program, i));
                multi = attributes.some(a => a.name === "a_texCoord" && a.type === gl.FLOAT_VEC3) &&
                    attributes.some(a => a.name === "a_color");
            }
            programs.set(program, multi ? 2 : Number(standard));
        }
        return programs.get(program);
    }
    let gui = guiKind(gl.getParameter(gl.CURRENT_PROGRAM));
    let total = 0, hud = 0, batches = 0;
    const originals = new Map();
    function observe(name, before) {
        const original = gl[name];
        originals.set(name, Object.getOwnPropertyDescriptor(gl, name));
        gl[name] = function (...args) {
            before(...args);
            return original.apply(this, args);
        };
    }
    const samples = [];
    try {
        observe("useProgram", program => { gui = guiKind(program); });
        for (const name of ["drawArrays", "drawElements", "drawArraysInstanced", "drawElementsInstanced"])
            observe(name, () => { ++total; if (gui) ++hud; if (gui === 2) ++batches; });
        let previous, lastDisplay = 0;
        let summary = "";
        while (live || samples.length < 120) {
            const now = await new Promise(requestAnimationFrame);
            if (total > 0) {
                if (previous !== undefined) {
                    if (!live) samples.push({guiDraws: hud, multiTextureDraws: batches, otherDraws: total - hud, frameMs: now - previous});
                    if (bench) {
                        samples.push([hud, batches, total - hud, now - previous]);
                        if (samples.length === 120) {
                            const median = i => {
                                const values = samples.map(s => s[i]).sort((a, b) => a - b);
                                return (values[59] + values[60]) / 2;
                            };
                            summary = "\nLast 120 frames (median)\nGUI " + median(0) + " (" + median(1) + " multi)  Other " +
                                median(2) + "\nFrame " + median(3).toFixed(1) + " ms";
                            samples.length = 0;
                        }
                    }
                    if (live && overlay && now - lastDisplay >= 250) {
                        overlay.textContent = "WebGL draws / frame\nGUI    " + hud + " (" + batches + " multi-texture)\nOther  " + (total - hud) +
                            "\nTotal  " + total + "\nFrame  " + (now - previous).toFixed(1) + " ms" + summary;
                        lastDisplay = now;
                    }
                }
                previous = now;
                total = hud = batches = 0;
            }
        }
    } finally {
        for (const [name, descriptor] of originals) {
            if (descriptor) Object.defineProperty(gl, name, descriptor);
            else delete gl[name];
        }
    }
    const results = {};
    for (const name of Object.keys(samples[0])) {
        const sorted = samples.map(s => s[name]).sort((a, b) => a - b);
        results[name] = {median: (sorted[59] + sorted[60]) / 2, p95: sorted[113]};
    }
    console.table(results);
    return results;
})()
