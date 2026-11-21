"""Exercise the local realm with real Chrome input. Requires Playwright and Chrome."""

import re
import time
from urllib.parse import urlsplit

from playwright.sync_api import sync_playwright


def verify(url, output, channel):
    output.mkdir(parents=True, exist_ok=True)
    with sync_playwright() as playwright:
        browser = playwright.chromium.launch(channel=channel)
        context = browser.new_context(
            viewport={"width": 1100, "height": 720}, service_workers="block"
        )
        page = context.new_page()
        external, errors, previous_logs = [], [], []
        host = urlsplit(url).netloc

        def record_request(request):
            dest = urlsplit(request.url)
            if dest.scheme in ("http", "https") and dest.netloc != host:
                external.append(request.url)

        context.on("request", record_request)
        page.on("websocket", lambda socket: external.append(socket.url))
        page.on("pageerror", lambda error: errors.append(str(error)))

        def read_log():
            return page.evaluate("""() => {try {return FS.readFile('/log.txt', {encoding:'utf8'})}
                                         catch {return ''}}""")

        def wait(marker):
            print(f"Waiting for {marker}", flush=True)
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                log = read_log()
                if (
                    "<Runtime error>" in log
                    or errors
                    or log.count("HAQI_TEST_ENTER") > 1
                ):
                    break
                if marker in log:
                    return log
                page.wait_for_timeout(200)
            page.screenshot(path=str(output / "failure.png"))
            raise AssertionError(
                f"Missing {marker}. See {output}/engine.log and {errors}"
            )

        def screenshot(name):
            page.screenshot(path=str(output / (name + ".png")))

        def quest(action):
            page.evaluate("(name) => FS.writeFile('temp/check-' + name, '1')", action)

        def login():
            page.goto(url)
            page.get_by_role("button", name="使用测试角色登录哈奇镇").click()
            wait("HAQI_TEST_SPAWN 20057.70 0.78 19696.45 guests=36")
            wait("CHECK inventory")

        try:
            assert context.request.get(url + "/bags/12").status == 403
            login()
            page.wait_for_timeout(1200)
            screenshot("spawn")
            page.mouse.click(1035, 214)
            wait("HAQI_TEST_FIRE_BEGIN")
            page.wait_for_timeout(1200)
            screenshot("fire")
            wait("HAQI_TEST_FIRE_COMPLETE hp=75")
            page.mouse.click(1035, 245)
            match = re.search(
                r"HAQI_TEST_PICK ([\d.]+) ([\d.]+)", wait("HAQI_TEST_PICK ")
            )
            x, y = map(float, match.groups())
            page.wait_for_timeout(600)
            page.mouse.click(1100 * x, 720 * y)
            page.wait_for_timeout(1000)
            wait("CHECK shop")
            screenshot("merchant")
            page.keyboard.press("Escape")
            page.wait_for_timeout(350)
            page.keyboard.press("KeyB")
            log = wait("CHECK equipment")
            match = re.search(r"CHECK bag_tab ([\d.]+) ([\d.]+)", log)
            x, y = map(float, match.groups())
            page.mouse.click(1100 * x, 720 * y)
            page.wait_for_timeout(700)
            wait("CHECK bag_open")
            screenshot("bag")
            quest("TryAccept")
            wait("CHECK quest3")
            previous_logs.append(read_log())
            page.reload()
            wait("CHECK quest3")
            quest("TryDrop")
            wait("CHECK quest5")
            assert (
                context.request.get(
                    url + "/fs/file?path=haqi-test/session.json"
                ).json()["quests"][0]["QuestState"]
                == 5
            )
            previous_logs.append(read_log())
            login()
            assert (
                context.request.get(
                    url + "/fs/file?path=haqi-test/session.json"
                ).json()["quests"]
                == []
            )
            wait("CHECK stable")
            assert not errors and not external, (errors, external)
            print(
                "PASS: local login, birth point, 36 guests, fire, NPC shop, bag, quest accept/reload/drop and fresh login"
            )
        finally:
            (output / "engine.log").write_text(
                "\n".join(previous_logs + [read_log()]), encoding="utf-8"
            )
            browser.close()
