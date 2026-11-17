let g_default_camera_id = "__default_camera_id__";
let g_current_camera_id = undefined;
let g_camera_devices = {};
let g_cameras = {};
let g_current_camera = undefined;

class Camera {
    constructor() {
        this.m_img = undefined;
        this.m_video = undefined;
        this.m_canvas = undefined;
        this.m_context = undefined;
        this.m_image_width = 400;
        this.m_image_height = 300;
        this.m_device_id = undefined;
        this.m_camera_id = undefined;
    }

    async Start(camera_id, width, height, camera_device_id, camera_url) {
        await this.RefreshCameraDeviceSelectOptions();

        camera_id = camera_id || g_default_camera_id;
        if (g_cameras[camera_id] !== undefined) return;
        let camera_index = parseInt(camera_device_id || camera_id);
        let device_id = undefined;
        let unused_devices = [];

        if (!camera_url) {
            for (let key in g_camera_devices) {
                let device = g_camera_devices[key];
                if (device.index == camera_index) {
                    device_id = device.deviceId;
                    device.used = true;
                    break;
                }
                if (!device.used) {
                    unused_devices.push(device);
                }
            }

            if (!device_id && unused_devices.length > 0) {
                device_id = unused_devices[0].deviceId;
                unused_devices[0].used = true;
            }
        }

        this.m_camera_id = camera_id;
        this.m_image_width = Math.floor(width || this.m_image_width);
        this.m_image_height = Math.floor(height || this.m_image_height);

        if (GetSystem() == "android") {
            this.m_image_width = window.innerWidth;
            this.m_image_height = window.innerHeight;
        }

        const body = document.body;
        const bodyWidth = body.clientWidth;
        const bodyHeight = body.clientHeight;
        const scale_x = bodyWidth / this.m_image_width;
        const scale_y = bodyHeight / this.m_image_height;
        body.style.transform = `scale(${scale_x}, ${scale_y})`;
        body.style.transformOrigin = "left top";
        
        const canvas = document.createElement("canvas");
        canvas.id = "canvas_" + this.m_camera_id;
        canvas.style.position = "absolute";
        canvas.style.left = "0px";
        canvas.style.top = "0px";
        canvas.style.visibility = "hidden";
        canvas.width = this.m_image_width;
        canvas.height = this.m_image_height;
        document.body.appendChild(canvas);
        this.m_canvas = canvas;
        this.m_context = canvas.getContext('2d');

        if (camera_url) {
            const img = document.createElement('img');
            img.id = "img_" + this.m_camera_id;
            img.style.position = "absolute";
            img.style.left = "0px";
            img.style.top = "0px";
            img.style.visibility = "hidden";
            img.width = this.m_image_width;
            img.height = this.m_image_height;
            img.src = camera_url;
            img.crossOrigin = "anonymous";
            document.body.appendChild(img);
            this.m_img = img;
        } else {
            const video = document.createElement("video");
            video.setAttribute('playsinline', '');
            video.setAttribute('webkit-playsinline', '');
            video.id = "video_" + this.m_camera_id;
            video.style.position = "absolute";
            video.style.left = "0px";
            video.style.top = "0px";
            // video.style.visibility = "visible";
            video.style.visibility = "hidden";
            video.width = this.m_image_width;
            video.height = this.m_image_height;
            document.body.appendChild(video);
            this.m_video = video;
            this.SetDeviceId(device_id);
        }

        g_cameras[camera_id] = this;

        const send_png_image = () => {
            this.m_context.drawImage(this.m_img || this.m_video, 0, 0, this.m_image_width, this.m_image_height);
            if (window.NPLJSInstance) window.NPLJSInstance.SendMsg("CameraSnapReply", this.m_canvas.toDataURL("image/png"));
        };

        (this.m_img || this.m_video).addEventListener("click", send_png_image);

        send_png_image();
        this.RefreshCurrentCameraSelectOptions();

        if (window.NPLJSInstance) window.NPLJSInstance.SendMsg("CameraStart", { camera_id: this.m_camera_id, width: this.m_image_width, height: this.m_image_height });
    }

    SetWidthHeight(width, height) {
        width = Math.floor(width || this.m_image_width);
        height = Math.floor(height || this.m_image_height);
        if (width == this.m_image_width && height == this.m_image_height) return;

        this.m_image_width = width;
        this.m_image_height = height;

        if (this.m_video) {
            this.m_video.width = this.m_image_width;
            this.m_video.height = this.m_image_height;
        }

        if (this.m_img) {
            this.m_img.width = this.m_image_width;
            this.m_img.height = this.m_image_height;
        }

        if (this.m_canvas) {
            this.m_canvas.width = this.m_image_width;
            this.m_canvas.height = this.m_image_height;
        }
    }

    Stop() {
        if (this.m_video) {
            this.m_video.pause();
            this.m_video.srcObject = null;
            document.body.removeChild(this.m_video);
        }
        if (this.m_canvas) {
            this.m_canvas.parentNode.removeChild(this.m_canvas);
        }
        if (this.m_device_id) {
            g_camera_devices[this.m_device_id].used = false;
        }
        g_camera_devices[this.m_camera_id] = undefined;

        if (g_current_camera == this) g_current_camera = undefined;
        this.RefreshCurrentCameraSelectOptions();
    }

    async RefreshCameraDeviceSelectOptions() {
        const camera_device_select = document.getElementById('camera_device_select');
        // 移除所有子元素
        while (camera_device_select.firstChild) {
            camera_device_select.removeChild(camera_device_select.firstChild);
        }
        // 获取摄像头列表
        let devices = [];
        try {
            devices = await navigator.mediaDevices.enumerateDevices();
            devices = devices.filter((device) => { return device.kind === 'videoinput' && device.deviceId; });
            console.log(devices);
        } catch (e) {
            console.log(e);
            console.error('无法获取摄像头列表：', e);
        }

        g_camera_devices = {};
        for (let i = 0; i < devices.length; i++) {
            const option = document.createElement('option');
            const device = devices[i];
            const deviceId = device.deviceId;

            option.value = deviceId;
            option.text = device.label || deviceId;

            g_camera_devices[deviceId] = { deviceId: deviceId, used: false, index: i };
            camera_device_select.appendChild(option);
        }

        for (let key in g_cameras) {
            const device_id = g_cameras[key].m_device_id;
            if (g_camera_devices[device_id]) {
                g_camera_devices[device_id].used = true;
            }
        }
    }

    RefreshCurrentCameraSelectOptions() {
        const current_camera_select = document.getElementById('current_camera_select');
        // 移除所有子元素
        while (current_camera_select.firstChild) {
            current_camera_select.removeChild(current_camera_select.firstChild);
        }

        for (let key in g_cameras) {
            const option = document.createElement('option');
            option.value = key;
            option.text = key;
            current_camera_select.appendChild(option);

            if (g_current_camera == undefined) Camera.SetCurrentCamera(g_cameras[key]);
        }
    }

    static SetCurrentCamera(camera) {
        if (g_current_camera == camera) return;
        if (g_current_camera) {
            (g_current_camera.m_img || g_current_camera.m_video).style.visibility = "hidden";
        }
        g_current_camera = camera;
        if (g_current_camera) {
            (g_current_camera.m_img || g_current_camera.m_video).style.visibility = "visible";

            const current_camera_select = document.getElementById('current_camera_select');
            current_camera_select.value = g_current_camera.m_camera_id;
            const camera_device_select = document.getElementById('camera_device_select');
            camera_device_select.value = g_current_camera.m_device_id;
        }
    }

    SetDeviceId(device_id) {
        if (!this.m_video) return;
        if (this.m_device_id === device_id && device_id) return;

        if (this.m_device_id) {
            g_camera_devices[this.m_device_id].used = false;
        }

        this.m_device_id = device_id;

        if (this.m_device_id) {
            g_camera_devices[this.m_device_id].used = true;
        }

        // 打开所选摄像头并显示视频流
        navigator.mediaDevices.getUserMedia({
            video: { deviceId: { exact: this.m_device_id } }
        }).then((stream) => {
            if (this.m_video) {
                this.m_video.srcObject = stream;
                this.m_video.play();
            }
        }).catch(function (error) {
            console.error('无法打开摄像头：', error);
        });
    }

    GetImageData(format, x, y, width, height) {
        this.m_context.drawImage(this.m_img || this.m_video, 0, 0, this.m_image_width, this.m_image_height);

        if (format !== "raw" && format !== "rgbstring") {
            const base64_image_data = this.m_canvas.toDataURL("image/" + format);
            return { width: this.m_image_width, height: this.m_image_height, data: base64_image_data, camera_id: this.m_camera_id };
        }

        x = x || 0;
        y = y || 0;
        width = width || this.m_image_width;
        height = height || this.m_image_height;
        const isrgbstring = format === "rgbstring";
        const imageData = this.m_context.getImageData(x, y, width, height);
        const pixelData = imageData.data;
        const pixels = [];
        let pixelString = '';
        for (let i = 0; i < pixelData.length; i += 4) {
            const red = pixelData[i];
            const green = pixelData[i + 1];
            const blue = pixelData[i + 2];
            const alpha = pixelData[i + 3];
            if (isrgbstring) {
                pixelString += "#" + ((red << 16) | (green << 8) | blue).toString(16).padStart(6, '0');
            }
            else {
                pixels.push(red);
                pixels.push(green);
                pixels.push(blue);
                pixels.push(alpha);
            }
        }
        return { data: isrgbstring ? pixelString : pixels, width: width, height: height, camera_id: this.m_camera_id };
    }
}

function registerCameraHandlers() {
    if (window.NPLJSInstance) {
        NPLJSInstance.OnMsg("CameraSnap", (msgdata, msgid) => {
            const camera_id = msgdata.camera_id || g_default_camera_id;
            let camera = g_cameras[camera_id];
            if (camera == undefined) {
                camera = new Camera();
                camera.Start(camera_id, msgdata.width, msgdata.height, msgdata.device_id, msgdata.camera_url);
            }
            else {
                camera.SetWidthHeight(msgdata.width, msgdata.height);
            }
        });

        NPLJSInstance.OnMsg("CameraSnapData", async (msgdata, msgid) => {
            const camera_id = msgdata.camera_id || (g_current_camera ? g_current_camera.m_camera_id : g_default_camera_id);
            const camera = g_cameras[camera_id];

            if (!camera || !camera.m_context) {
                (new Camera()).Start(camera_id, msgdata.width, msgdata.height, msgdata.camera_url);
                return NPLJSInstance.SendMsg("CameraSnapDataReply", { width: 0, height: 0, data: [], camera_id: camera_id }, msgid);
            }
            const data = camera.GetImageData(msgdata.format, msgdata.x, msgdata.y, msgdata.width, msgdata.height);
            NPLJSInstance.SendMsg("CameraSnapDataReply", data, msgid);
        });
    }
}

registerCameraHandlers();

const camera_refresh_container = document.getElementById('camera_refresh_container');
camera_refresh_container.addEventListener('mouseenter', function () {
    camera_refresh_container.style.opacity = '1';
});

camera_refresh_container.addEventListener('mouseleave', function () {
    camera_refresh_container.style.opacity = '0';
});

// 监听下拉菜单选择事件
const camera_device_select = document.getElementById('camera_device_select');
camera_device_select.addEventListener('change', function (event) {
    const device_id = event.target.value;
    if (g_current_camera) g_current_camera.SetDeviceId(device_id);
});

const camera_show_container = document.getElementById('camera_show_container');
camera_show_container.addEventListener('mouseenter', function () {
    camera_show_container.style.opacity = '1';
});

camera_show_container.addEventListener('mouseleave', function () {
    camera_show_container.style.opacity = '0';
});

// 监听下拉菜单选择事件
const current_camera_select = document.getElementById('current_camera_select');
current_camera_select.addEventListener('change', function (event) {
    const camera_id = event.target.value;
    Camera.SetCurrentCamera(g_cameras[camera_id]);
});  