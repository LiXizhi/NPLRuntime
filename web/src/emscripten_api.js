
const EmscriptenInstanceApi = window.EmscriptenInstanceApi;

EmscriptenInstanceApi.OnMsg("GetParentWindowURL", function (msgdata, msgid) {
    const win = window.parent || window;
    const url = win.location.href;
    EmscriptenInstanceApi.SendMsg("GetParentWindowURLReply", { url: url }, msgid);
})

EmscriptenInstanceApi.OnMsg("OpenFileDialog", function (msgdata, msgid) {
    msgdata = msgdata || {};
    const input = document.createElement("input");
    input.type = "file";
    input.multiple = msgdata.multiple; // true or false
    input.accept = msgdata.accept; // MIME "image/*" or "image/*, audio/*"
    input.addEventListener("change", function () {
        let handle_count = input.files.length;
        let filepaths = [];
        if (input.files.length > 0) {
            for (let i = 0; i < input.files.length; i++) {
                (function(file){
                    const reader = new FileReader();
                    reader.onload = function () {
                        const data = new Uint8Array(reader.result);
                        const filename = "emscripten_" + file.name;
                        const filepath = "/tmp/" + filename;  // FS.cwd() + "/" + filename;
                        try {
                            FS.writeFile(filepath, data, { encoding: "binary" });
                            console.log("文件已成功保存到 Emscripten VFS 中:", filepath);
                            filepaths.push(filepath);
    
                        } catch (e) {
                            console.error("保存文件到 Emscripten VFS 失败:", e);
                        }
                        handle_count = handle_count - 1;
                        if (handle_count == 0) EmscriptenInstanceApi.SendMsg("OpenFileDialogReply", { filepaths: filepaths }, msgid);
                    }
                    reader.onerror = function (event) {
                        console.error("读取文件失败:", event.target.error);
                        handle_count = handle_count - 1;
                        if (handle_count == 0) EmscriptenInstanceApi.SendMsg("OpenFileDialogReply", { filepaths: filepaths }, msgid);
                    };
                    reader.readAsArrayBuffer(file);
                })(input.files[i]);
            }
        } else {
            console.log("用户没有选择文件");
            EmscriptenInstanceApi.SendMsg("OpenFileDialogReply", { filepaths: filepaths }, msgid);
        }
    });
    input.click();
})
