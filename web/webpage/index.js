let isMobile = /iPhone|iPad|iPod|Android|Windows Phone/i.test(navigator.userAgent);
let video = undefined;
let canvas = undefined;
let context = undefined;
let image_width = 400;
let image_height = 300;
let image_url = "";
let current_msgid = undefined;
let autoplayMutedFallback = false;

function tryPlayVideo(el) {
    if (!el) return;
    const p = el.play();
    if (p && typeof p.catch === "function") {
        p.catch(function () {
            if (!el.muted) {
                el.muted = true;
                autoplayMutedFallback = true;
                const p2 = el.play();
                if (p2 && typeof p2.catch === "function") {
                    p2.catch(function () {});
                }
            }
        });
    }
}

window.addEventListener("message", function (ev) {
    if (ev && ev.data && ev.data.type === "paracraft_user_gesture" && video && autoplayMutedFallback) {
        autoplayMutedFallback = false;
        video.muted = false;
        tryPlayVideo(video);
    }
});

function StartWebPageSnap(width, height, url) {
    image_width = width || image_width;
    image_height = height || image_height;
    image_url = url || image_url;

    if (isMobile) {
        const local_video = document.getElementById("video");
        const local_canvas = document.getElementById("canvas");
        const local_context = local_canvas.getContext("2d", { willReadFrequently: true });
        local_video.crossOrigin = "anonymous";
        local_video.width = image_width;
        local_video.height = image_height;
        local_video.src = image_url;
        local_video.style.visibility = "hidden";
        local_canvas.width = image_width;
        local_canvas.height = image_height;

        if (window.parent && window.parent !== window && window.parent.postMessage) {
            window.parent.postMessage({
                is_paracraft_message: true,
                cmd: "enter_fullscreen",
            }, "*");

            document.onclick = function () {
                tryPlayVideo(local_video);
                local_video.style.visibility = "visible";
                video = local_video;
                canvas = local_canvas;
                context = local_context;
                document.onclick = undefined;

                window.parent.postMessage({
                    is_paracraft_message: true,
                    cmd: "exit_fullscreen",
                }, "*");

                setTimeout(function () {
                    const body = document.body;
                    const bodyWidth = body.clientWidth || image_width;
                    const bodyHeight = body.clientHeight || image_height;
                    const scale_x = bodyWidth / image_width;
                    const scale_y = bodyHeight / image_height;
                    body.style.transform = `scale(${scale_x}, ${scale_y})`;
                    body.style.transformOrigin = "left top";
                    if (window.NPLJSInstance) {
                        NPLJSInstance.SendMsg("WebPageSnapReply", {}, current_msgid);
                    }
                }, 100);
            };
        }
        return;
    }

    if (video !== undefined) {
        video.crossOrigin = "anonymous";
        video.width = image_width;
        video.height = image_height;
        if (image_url && video.getAttribute("data-raw-src") !== image_url) {
            video.setAttribute("data-raw-src", image_url);
            video.src = image_url;
            autoplayMutedFallback = false;
            video.muted = false;
        }
        tryPlayVideo(video);
        canvas.width = image_width;
        canvas.height = image_height;
        if (window.NPLJSInstance) {
            NPLJSInstance.SendMsg("WebPageSnapReply", {}, current_msgid);
        }
        return;
    }

    video = document.getElementById("video") || document.createElement("video");
    video.setAttribute("playsinline", "");
    video.setAttribute("webkit-playsinline", "");
    video.crossOrigin = "anonymous";
    video.loop = true;
    video.autoplay = true;
    video.style.position = "absolute";
    video.style.left = "0px";
    video.style.top = "0px";
    video.style.visibility = "visible";
    video.width = image_width;
    video.height = image_height;
    if (image_url) {
        video.setAttribute("data-raw-src", image_url);
        video.src = image_url;
    }
    if (!video.parentNode) {
        document.body.appendChild(video);
    }
    tryPlayVideo(video);

    canvas = document.getElementById("canvas") || document.createElement("canvas");
    canvas.style.position = "absolute";
    canvas.style.left = "0px";
    canvas.style.top = "0px";
    canvas.style.visibility = "hidden";
    canvas.width = image_width;
    canvas.height = image_height;
    context = canvas.getContext("2d", { willReadFrequently: true });
    if (!canvas.parentNode) {
        document.body.appendChild(canvas);
    }

    if (window.NPLJSInstance) {
        NPLJSInstance.SendMsg("WebPageSnapReply", {}, current_msgid);
    }
}

function registerWebPageHandlers() {
    if (!window.NPLJSInstance) {
        StartWebPageSnap();
        return;
    }
    NPLJSInstance.OnMsg("WebPageSnap", function (msgdata, msgid) {
        current_msgid = msgid;
        if (msgdata && typeof msgdata === "object") {
            image_width = msgdata.width || image_width;
            image_height = msgdata.height || image_height;
            StartWebPageSnap(image_width, image_height, msgdata.url);
        } else {
            StartWebPageSnap();
        }
    });

    NPLJSInstance.OnMsg("CameraSnapData", function (msgdata, msgid) {
        msgdata = (msgdata && typeof msgdata === "object") ? msgdata : {};
        if (!context || !video) {
            if (msgdata.url) {
                StartWebPageSnap(msgdata.width, msgdata.height, msgdata.url);
            }
            return NPLJSInstance.SendMsg("CameraSnapDataReply", { width: 0, height: 0, data: "" }, msgid);
        }

        if (video.paused && !video.ended) {
            tryPlayVideo(video);
        }
        if (video.readyState < 2 || !video.videoWidth || !video.videoHeight) {
            return NPLJSInstance.SendMsg("CameraSnapDataReply", { width: 0, height: 0, data: "" }, msgid);
        }

        try {
            context.drawImage(video, 0, 0, image_width, image_height);
        } catch (e) {
            return NPLJSInstance.SendMsg("CameraSnapDataReply", { width: 0, height: 0, data: "" }, msgid);
        }

        if (msgdata.format !== "raw" && msgdata.format !== "rgbstring") {
            const mimeFormat = msgdata.format === "jpg" ? "jpeg" : (msgdata.format || "png");
            const base64_image_data = canvas.toDataURL("image/" + mimeFormat);
            NPLJSInstance.SendMsg("CameraSnapDataReply", { width: canvas.width, height: canvas.height, data: base64_image_data }, msgid);
            return;
        }

        const isrgbstring = msgdata.format === "rgbstring";
        const x = msgdata.x || 0;
        const y = msgdata.y || 0;
        const w = msgdata.width || canvas.width;
        const h = msgdata.height || canvas.height;
        const imageData = context.getImageData(x, y, w, h);
        const pixelData = imageData.data;
        const pixels = [];
        let pixelString = "";
        for (let i = 0; i < pixelData.length; i += 4) {
            const red = pixelData[i];
            const green = pixelData[i + 1];
            const blue = pixelData[i + 2];
            const alpha = pixelData[i + 3];
            if (isrgbstring) {
                pixelString += "#" + ((red << 16) | (green << 8) | blue).toString(16).padStart(6, "0");
            } else {
                pixels.push(red, green, blue, alpha);
            }
        }
        NPLJSInstance.SendMsg("CameraSnapDataReply", { data: isrgbstring ? pixelString : pixels, width: w, height: h }, msgid);
    });
}

registerWebPageHandlers();
