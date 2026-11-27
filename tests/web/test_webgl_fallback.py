"""Run with Python Playwright and Chrome against a locally served Web Paracraft world.

    python3 tests/web/test_webgl_fallback.py '<world URL without a renderer override>'
    python3 tests/web/test_webgl_fallback.py --missing-bgra '<world URL without a renderer override>'
"""
import argparse
from urllib.parse import parse_qs, urlsplit

from playwright.sync_api import sync_playwright


def verify_fallback(url, missing_bgra=False):
    assert "renderer" not in parse_qs(urlsplit(url).query), "Use automatic renderer selection"
    with sync_playwright() as playwright:
        browser = playwright.chromium.launch(channel="chrome", args=[
            "--disable-webgl2", "--use-gl=angle", "--use-angle=swiftshader",
            "--enable-unsafe-swiftshader", "--enable-unsafe-webgpu",
        ])
        try:
            page = browser.new_page()
            failures = []

            def check_render_message(message):
                if any(error in message.text for error in (
                    "Failed to compile shader", "skipping unsupported OpenGL technique",
                    "failed creating m_waveReflectionRT", "WebGL: INVALID_ENUM: renderbufferStorage",
                    "conflicting DEPTH/STENCIL",
                )):
                    failures.append(message.text)

            page.on("console", check_render_message)
            page.on("pageerror", lambda error: failures.append(str(error)))
            if missing_bgra:
                page.add_init_script("""(() => {
                    for (const name of ['createRenderPipeline', 'createRenderPipelineAsync']) {
                        const create = GPUDevice.prototype[name];
                        GPUDevice.prototype[name] = function (desc) {
                            const buffers = desc.vertex.buffers || [];
                            if (buffers.some(b => b?.attributes.some(a => a.format === 'unorm8x4-bgra'))) {
                                window.bgraRejected = true;
                                const error = new TypeError("Unsupported GPUVertexFormat: unorm8x4-bgra");
                                if (name === 'createRenderPipelineAsync') return Promise.reject(error);
                                throw error;
                            }
                            return create.call(this, desc);
                        };
                    }
                })()""")
            else:
                page.add_init_script("""navigator.gpu.requestAdapter = async () => {
                    window.adapterUnavailable = true;
                    return null;
                };""")
            page.add_init_script("""(() => {
                // Exercise the desktop Windows WebGPU selection policy on any test host.
                Object.defineProperty(navigator.userAgentData, 'platform', {value: 'Windows'});
                window.fallbackProbe = {draws: 0};
                const gl = WebGLRenderingContext.prototype;
                const checkFramebufferStatus = gl.checkFramebufferStatus;
                gl.checkFramebufferStatus = function (target) {
                    const status = checkFramebufferStatus.call(this, target);
                    if (status === this.FRAMEBUFFER_COMPLETE && this.getParameter(this.FRAMEBUFFER_BINDING)) {
                        const attachment = this.getFramebufferAttachmentParameter(
                            target, this.DEPTH_STENCIL_ATTACHMENT, this.FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE);
                        if (attachment === this.RENDERBUFFER) fallbackProbe.packedTargetComplete = true;
                    }
                    return status;
                };
                for (const name of ['drawArrays', 'drawElements']) {
                    const draw = gl[name];
                    gl[name] = function (...args) {
                        const result = draw.apply(this, args);
                        if (this.canvas.id === 'canvas') fallbackProbe.draws++;
                        return result;
                    };
                }
            })()""")
            with page.expect_console_message(
                lambda message: "SendWorldLoadingFinished" in message.text, timeout=120000
            ), page.expect_console_message(
                lambda message: "ParaRHI initialized backend:" in message.text, timeout=90000
            ) as selected:
                page.goto(url, wait_until="domcontentloaded")
            expected_probe = "window.bgraRejected" if missing_bgra else "window.adapterUnavailable"
            assert page.evaluate(expected_probe), f"{expected_probe} did not run"
            assert "backend: OpenGL" in selected.value.text, selected.value.text
            page.wait_for_function("window.fallbackProbe.draws > 0", timeout=90000)
            assert not failures, "\n".join(failures)
            assert page.evaluate("window.fallbackProbe.packedTargetComplete"), "Missing packed depth/stencil target"
            version = page.locator("#canvas").evaluate("""canvas => {
                const gl = canvas.getContext('webgl');
                return gl && gl.getParameter(gl.VERSION);
            }""")
            assert version and version.startswith("WebGL 1.0"), version
            result = page.evaluate("""() => new Promise(resolve => EmscriptenInstance.ExecuteLuaCode(`
                local attr = ParaTerrain.GetBlockAttributeObject()
                GameLogic.RunCommand("/shader 0")
                local before = attr:GetField("BlockRenderMethod", -1)
                GameLogic.RunCommand("/shader 2")
                return {before=before, after=attr:GetField("BlockRenderMethod", -1),
                        level=GameLogic.GetShaderManager():GetQualityPreset(),
                        advanced=attr:GetField("CanUseAdvancedShading", false)}
            `, {}, resolve))""")
            assert result.get("success"), result
            assert result["result"] == {"before": 0, "after": 1, "level": 1, "advanced": False}, result
            print(f"Automatic fallback rendered with {version} and /shader 2 selected shader 1")
        finally:
            browser.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("url")
    parser.add_argument("--missing-bgra", action="store_true", help="Emulate a WebGPU device that rejects packed BGRA vertex colors")
    args = parser.parse_args()
    verify_fallback(args.url, args.missing_bgra)
