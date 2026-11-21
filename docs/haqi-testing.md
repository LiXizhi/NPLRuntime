# Running and testing Haqi (魔法哈奇)

Run commands from the NPLRuntime checkout. Pick the entry point for the behavior you are changing.

| What you are testing | Source and entry point |
|---|---|
| Native engine, rendering or NPL scripts | This engine checkout and the sibling `ParaEngine/paraworld/script/` tree. [Native launchers and options](../tests/haqi/README.md#native-macos--windows) |
| Web game with a locally built engine | The sibling `webparacraft` checkout supplies `haqi1.html` and the generated game packages. The [local launcher](../tests/haqi/launch_haqi.py) serves those with this checkout's JS/WASM pair. |
| Deployed Web game | Open the game shell on the environment under test. [Dev](https://emscripten.keepwork.com/haqi1.html) · [Production](https://webparacraft.keepwork.com/haqi1.html) |
| Official homepage, downloads, guides and game handoff | KeepWork repository `official/open`, file `official/open/apps/haqi/haqi_web.md`. The public Haqi domain embeds this KeepWork page, which embeds the game after a click. [Repository](https://git.keepwork.com/official/open) · [Official homepage](https://haqi.keepwork.com/#official/open/apps/haqi/haqi_web) |

## Offline town during maintenance

During the 00:00–06:00 China time maintenance window, use the local test realm to enter the kids edition of Haqi Town. You need Python 3.9+, a WebGL2 browser, the sibling `../ParaEngine/paraworld` scripts and `../webparacraft` with its generated Haqi packages and matching JS/WASM. Override their locations with `--paraworld-root` and `--web-root`.

```bash
python3 tests/haqi/launch_offline.py --prepare   # once while public artwork is available
python3 tests/haqi/launch_offline.py             # works disconnected after preparation
```

Open [the local realm](http://127.0.0.1:8094/) and click **使用测试角色登录哈奇镇**. The launcher issues a temporary local session and the Aries world loader places the character at the configured birth point in `61HaqiTown`. Walk around, use the bottom dock, or click **去找米歇尔** and then click the merchant herself to open her shop. **训练：火系技能** targets one of 36 dressed stand-ins near spawn and plays the stock fire spell. Casts change the local target's HP. To start over, open the root URL and log in again. Refreshing the running game keeps that session's quest state.

The original Aries client runs the HUD, menus, quest tracker, town NPC scripts, shop display and inventory. The stock `paraworld.auth.AuthUser` handler consumes the loopback login response. The service supplies a synthetic account, pet, inventory, quest storage and a training target. The quest provider checks prerequisites and supports accepting and dropping quests. The crowd exercises character rendering. Purchases, reward settlement, other players' network traffic and live authentication require separate tests against the game service. Use the live login path to measure production startup. The tested package also logs missing definitions for six other card spells, including `PickPet.xml`. The training spell is `Fire_SingleAttack_Level1.xml`.

To run the repeatable browser check, install Chrome and the optional Python `playwright` package. The check launches its own realm, clicks through the shop and bag, accepts **启程火鸟岛** (63014), reloads, drops it, signs in afresh and checks the new session stays in town for 20 seconds. It records PNGs and the engine log under `build/haqi-offline-test/checks/`. Use another port if your manual session is open.

```bash
python3 tests/haqi/launch_offline.py --check --port 8095
```

The preparer reads public asset keys from the Haqi package manifest and the town and skill definitions. It reuses the native art cache when available and downloads missing art into the local build directory. Run `--prepare` again after updating the packages or the asset list. The generated archive and session data stay local. [Realm launcher](../tests/haqi/launch_offline.py) · [Asset recipe](../tests/haqi/offline/assets.txt) · [Browser check](../tests/haqi/verify_offline_world.py)

## Native launch and remembered login

Have Python 3, a native client build, the sibling Paraworld scripts and the generated Haqi packages in `../webparacraft/auto_generate_files/`. On macOS:

```bash
./build_linux.sh client --no-robotics
tests/haqi/run_haqi_mac.sh --renderer opengl
```

On Windows, build with `build_win32.bat` and run `tests\haqi\run_haqi_native.bat --renderer opengl`. Pass `--exe` to pin a particular build. On the first run, sign in with **Remember password**, choose a character and enter the server. Reuse the same runtime directory for later runs. The launcher requests saved sign-in and selects the last available character and server, with fallbacks to an available character and recommended server. Use `--manual-login` to change the selection.

The launcher prints the runtime path, asset manifest hash and client PID. Inspect `log.txt` and `stdout.log` there. To test package preparation alone, pass `--prepare-only --runtime-dir <scratch-directory>`. Keep login state in the local runtime. Omit credentials and token-bearing URLs from commands, shared logs and screenshots.

## Mobile movement and the Creative Space HUD

1. In Haqi Town, stand on open ground and keep the camera still. Drag the pad toward the bottom of the screen. The character should turn toward the camera and walk forward. Test left and right, rotate the camera and repeat.
2. Enter a local flat Creative Space from Haqi. Drag through all eight directions, cross the center from left to right, then drag back to the center and hold. Movement should follow the drag and stop at the center. While moving, tap jump with a second finger and release it. The first finger should still control movement.
3. On the first entry after launch, check that the loading screen covers the transition and the spawn terrain is visible when it closes. For timing comparisons, use the same client, account, server and local world in fresh processes. Record the interval from `WorldCommon.OpenWorld` to `Game.OnLogin finished`, then verify the spawn chunk separately.
4. In landscape, check the compact bottom hotbar, including the last slot and both action buttons. The Haqi dock should sit to the right of the hotbar on the same bottom row, with space between the last action button and the dock. Tap **系统** on the dock and **返回公共世界** in the menu. Verify the plaza renders, `ParaWorld.GetWorldDirectory()` contains `61HaqiTown`, and the joystick works. In browser automation, dispatch touch down and up on the visible controls and verify the destination. Calling the exit function directly skips the failing input path. In a fresh browser context, emulate a mobile user agent and touch from startup, resize the canvas in the plaza, and repeat the round trip. Check for WebAssembly errors during resizing and world changes. In a separate desktop browser context, verify the joystick and jump button stay hidden in both worlds.

5. Repeat the return as soon as the spawn terrain and system button appear. In the public world, observe at least six seconds of advancing frames with the loading page closed. A single visible frame can miss a Creative Space timer that reopens the loading page after the return. On macOS, run this once with `--renderer metal` and once with `--renderer opengl`, resizing the window in each world.

For a native desktop spot check, enter the plaza first, then run the following in the NPL console of a disposable session. This enables the touch UI for that session. Record the host and input method with your results. A phone run also exercises the OS touch delivery and screen layout.

```lua
System.options.IsTouchDevice = true
System.options.IsMobilePlatform = true
ParaCamera.GetAttributeObject():SetField("IsShiftMoveSwitched", true)
NPL.load("(gl)script/apps/Aries/Desktop/Dock/AriesMobilePage.lua")
MyCompany.Aries.Dock.AriesMobilePage.ShowPage(true)
```

## Compare C++ and NPL in the same plaza

1. Start the native launcher with `--renderer opengl --profile-unlimited-fps` for **both** builds. The default remains capped according to the host policy. For a live switch use `ParaEngine.GetAttributeObject():SetField("ProfileUnlimitedFPS", true)` in the NPL console. Set it back to `false` after the run.
2. Enter the same character and server and verify `ParaWorld.GetWorldDirectory()` contains `61HaqiTown`. Record player position, the camera's `Eye position` and `Lookat position`, resolution, HUD panels and renderer. Keep the window foreground, wait for nearby assets and characters to settle, and record engine/script revisions and asset manifest hash.
3. Measure throughput with both profilers stopped. Read `ParaScene.GetAttributeObject():GetField("FrameNumber", -1)` at each end of a timed window. Divide the frame delta by elapsed monotonic seconds for FPS, or divide elapsed milliseconds by the frame delta for ms/frame. Repeat before → after → after → before. Save matching screenshots and compare static HUD text and icons separately from animated players.
4. Sample C++ and NPL in separate windows. On macOS the native launcher writes `launch.pid`. While the default runtime's client is in the plaza, run:

```bash
sample "$(cat build/haqi-mac-runtime/launch.pid)" 12 1 -file build/haqi-plaza-native.sample.txt
```

For NPL on the LuaJIT client, execute the following in the **main NPL state**. It records VM state and up to eleven line-level frames, requests a 1 ms sampling interval, and writes counts when the timer stops it. The output is relative to the native runtime's working directory. Give each capture its own filename before comparing builds. [JITProfiler implementation and mode flags](../npl_packages/main/script/ide/Debugger/JITProfiler.lua)

```lua
local profiler = NPL.load("script/ide/Debugger/JITProfiler.lua")
profiler.start("vl11ri1m0", "haqi-plaza-npl.txt")
commonlib.TimerManager.SetTimeout(function() profiler.stop() end, 35000)
```

Record `jit.status()` and `debug.gethook()` for each capture. A high LuaJIT C-state count can include native calls and idle time. Use the native main-thread trace to distinguish rendering work from presentation waits. The historical reports give reference camera coordinates, sampling windows, HUD crops and the limits of each measurement. [Uncapped plaza comparison](../tests/haqi/plaza-profile-2026-11-20.md) · [HUD, ParaX and OpenGL follow-up](../tests/haqi/plaza-hud-batching-2026-11-20.md)

## Local Web game and regression fixtures

Build with `scripts/dev_web.sh build` (Windows: `scripts/dev_web.ps1 build`). The launcher expects the matching `ParaCraftSingleThread.js` and `.wasm` under `web/auto_generate_files/`. Use `--engine-dir` for a different build. Keep the loopback server running and open the URL it prints. Each command below selects a separate test:

```bash
python3 tests/haqi/launch_haqi.py --renderer webgl --draw-stats --no-browser
python3 tests/haqi/launch_haqi.py --test-missing-region --no-browser
python3 tests/haqi/launch_haqi.py --test-gui-batching --draw-stats --no-browser
```

| Default port | What to verify |
|---|---|
| 8091, live game | Sign in, select the character and server, then enter the plaza. Verify textured ground, buildings, avatars and the HUD. For WebGL draws append `&bench=1` to the printed draw-stats URL and allow two 120-frame groups after changing the UI. Use `&sceneprofile=1` to capture one frame separately. |
| 8092, missing terrain region | Look for `TERRAIN_REGION_CHECK_PASS` in the browser console. A trap or `TERRAIN_REGION_CHECK_FAIL` identifies a failed run. |
| 8093, offline HUD | Compare the same camera and canvas size. Inspect the overlapping translucent sprites, Chinese glyphs, clipped images and depth-tested overhead labels. |

For a deployed game, repeat the signed-in plaza check in a fresh browser context on each environment being tested. Save a screenshot after the scene renders. If a world is blank, inspect the console and network requests for the engine pair, game packages and world assets. Record the failing URL without its login token. A loaded shell or a successful sign-in only verifies that stage. [Detailed browser checks and fixtures](../tests/haqi/README.md#browser-tests)

A previous empty-world regression occurred after the world archive downloaded, when the client moved a file between Emscripten mounts. If the download succeeds but the expected world files are missing, trace the move into the writable world directory. [Cross-mount move regression cases](../NPLRuntime/tests/WebFileMoveTest.cpp)

## Official homepage acceptance

Edit the KeepWork Markdown in the `official/open` checkout. Its `@Title` block owns the header, `@Html → html.srcdoc` contains the landing page, and `@Permission` controls the existing login and pass prompt. The page's artwork is stored alongside the Markdown. The homepage and game shell have separate release paths.

- **Responsive layout.** At widths 320, 390, 768 and 1280 CSS pixels, verify “进入网页版” appears first. The next row contains Windows, 安卓 and TapTap in that order. Check for horizontal overflow. Expand the 游戏攻略 topics, follow the attributed links, and make sure the 岛上公告 section still loads.
- **Downloads.** In browser network tools, click each direct-download button. The dialog should identify the platform, exact filename, version and size before any EXE or APK request. Cancel and press Esc in separate attempts, then confirm and check that exactly the selected package is requested. When replacing a package, read the Windows installer file version or the APK's `versionName` and `versionCode`, and recheck the byte size. Verify that TapTap opens the [Magic Haqi game listing](https://www.taptap.cn/app/790136).
- **Game handoff.** On the live site, verify the normal visitor login prompt. With an authorized account and access, click “进入网页版”, check the game frame loads `haqi1.html`, enter the plaza, then exercise fullscreen, 返回官网 and 继续游戏. Record visitor-page verification separately if that is as far as the session gets.
- **Images and publication.** Inspect the homepage's own logo and hero requests. Both should decode and return `image/webp`. For a page update, compare the published Markdown and artwork with the intended revision, then open the public URL in a fresh browser context to verify the nested page.

To exercise the homepage controls locally before publishing, render the actual `srcdoc` from your sibling `open` checkout. This preview uses Python 3 and PyYAML and writes only to `build/`:

```bash
python3 - <<'PY'
from pathlib import Path
import yaml

page = Path("../open/official/open/apps/haqi/haqi_web.md").read_text(encoding="utf-8")
block = page.split("```@Html\n", 1)[1].split("\n```", 1)[0]
preview = Path("build/haqi-home-preview")
preview.mkdir(parents=True, exist_ok=True)
(preview / "index.html").write_text(yaml.safe_load(block)["html"]["srcdoc"], encoding="utf-8")
PY
python3 -m http.server 8094 --bind 127.0.0.1 --directory build/haqi-home-preview
```

Open [the local preview](http://127.0.0.1:8094/). It exercises the page content with its configured public assets. Use the official URL to test the KeepWork header, login, pass and signed-in game handoff.

For an automated read-only check, Python Playwright and an installed Google Chrome can inspect the published page through both iframe layers. This checks the visitor's page payload at phone width, even when the host presents its login dialog:

```python
from playwright.sync_api import sync_playwright

with sync_playwright() as playwright:
    browser = playwright.chromium.launch(channel="chrome")
    page = browser.new_page(viewport={"width": 390, "height": 844}, is_mobile=True)
    page.goto("https://haqi.keepwork.com/#official/open/apps/haqi/haqi_web",
              wait_until="domcontentloaded")
    home = page.frame_locator("iframe").first.frame_locator("iframe[srcdoc]").first
    home.locator("#play").wait_for(state="attached")
    assert home.locator("#play").inner_text() == "进入网页版"
    assert home.locator(".downloads > *").count() == 3
    assert home.locator("#guides-heading").inner_text() == "游戏攻略"
    assert home.locator("#game-frame").get_attribute("src") is None
    browser.close()
```
