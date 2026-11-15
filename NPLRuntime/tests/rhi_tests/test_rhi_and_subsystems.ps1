param(
    [Parameter(Mandatory = $true)]
    [string]$BinDir
)

$ErrorActionPreference = "Stop"
$env:PARAENGINE_NO_CRASH_DIALOG = "1"

$clientExe = Join-Path $BinDir "ParaEngineClient.exe"
if (-not (Test-Path $clientExe)) {
    throw "ParaEngineClient.exe not found at $clientExe"
}

Add-Type -AssemblyName System.Drawing

function Compute-PSNR {
    param([string]$PathA, [string]$PathB)
    $bmpA = [System.Drawing.Bitmap]::new($PathA)
    $bmpB = [System.Drawing.Bitmap]::new($PathB)
    try {
        $w = $bmpA.Width
        $h = $bmpA.Height
        $rect = [System.Drawing.Rectangle]::new(0, 0, $w, $h)
        $dataA = $bmpA.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        $dataB = $bmpB.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        try {
            $bytes = [Math]::Abs($dataA.Stride) * $h
            $bufA = New-Object byte[] $bytes
            $bufB = New-Object byte[] $bytes
            [System.Runtime.InteropServices.Marshal]::Copy($dataA.Scan0, $bufA, 0, $bytes)
            [System.Runtime.InteropServices.Marshal]::Copy($dataB.Scan0, $bufB, 0, $bytes)

            $mseSum = 0.0
            $count = 0L
            for ($y = 0; $y -lt $h; $y++) {
                $rowOff = $y * [Math]::Abs($dataA.Stride)
                for ($x = 0; $x -lt $w; $x++) {
                    $idx = $rowOff + $x * 4
                    $db = [double]$bufA[$idx + 0] - [double]$bufB[$idx + 0]
                    $dg = [double]$bufA[$idx + 1] - [double]$bufB[$idx + 1]
                    $dr = [double]$bufA[$idx + 2] - [double]$bufB[$idx + 2]
                    $mseSum += ($dr * $dr + $dg * $dg + $db * $db)
                    $count += 3
                }
            }
            $mse = $mseSum / [double]$count
            if ($mse -le 0.0000001) {
                return 99.99
            }
            return [Math]::Round(10.0 * [Math]::Log10((255.0 * 255.0) / $mse), 2)
        } finally {
            $bmpA.UnlockBits($dataA)
            $bmpB.UnlockBits($dataB)
        }
    } finally {
        $bmpA.Dispose()
        $bmpB.Dispose()
    }
}

$luaHarness = @'
local tick = 0
local outFile = ParaEngine.GetAppCommandLineByParam("out", "_ctest_out.txt")
local shotFile = ParaEngine.GetAppCommandLineByParam("shot", "_ctest_shot.bmp")
local expectBackend = ParaEngine.GetAppCommandLineByParam("expect_backend", "")
local testSubsystems = ParaEngine.GetAppCommandLineByParam("test_subsystems", "0")

local function fail(reason)
    local f = ParaIO.open(outFile, "w")
    if f:IsValid() then
        f:WriteString("FAIL: " .. tostring(reason) .. "\n")
        f:close()
    end
    ParaGlobal.Exit(1)
end

local function make_panel(name, x, y, w, h, colorStr)
    local p = ParaUI.CreateUIObject("container", name, "_lt", x, y, w, h)
    p.background = "Texture/whitedot.png"
    p:GetTexture("background").color = colorStr
    p:AttachToRoot()
    return p
end

local function main_loop()
    tick = tick + 1
    if tick == 1 then
        make_panel("_q_tl", 0,   0,   320, 240, "220 40 40 255")
        make_panel("_q_tr", 320, 0,   320, 240, "40 200 60 255")
        make_panel("_q_bl", 0,   240, 320, 240, "40 90 230 255")
        make_panel("_q_br", 320, 240, 320, 240, "235 210 40 255")
        local box = make_panel("_q_box", 160, 170, 320, 140, "20 25 35 255")

        local lbl1 = ParaUI.CreateUIObject("text", "_q_txt1", "_lt", 20, 25, 280, 36)
        lbl1.text = "ParaRHI Unified CTest"
        lbl1.font = "System;18;bold"
        lbl1:GetFont("text").color = "255 255 255"
        box:AddChild(lbl1)

        ParaMovie.SetCaptureGUI(true)
    elseif tick == 8 then
        local stats = ParaEngine.GetStats(0)
        if not stats or (expectBackend ~= "" and string.find(stats, expectBackend, 1, true) == nil) then
            return fail("Backend mismatch. Expected '" .. tostring(expectBackend) .. "', got: " .. tostring(stats))
        end

        local okShot = ParaMovie.TakeScreenShot(shotFile)
        if not okShot then
            return fail("TakeScreenShot failed for " .. tostring(shotFile))
        end

        local isLowEnd = ParaEngine.GetAttributeObject():GetField("IsLowEndDevice", false)
        if type(isLowEnd) ~= "boolean" then
            return fail("IsLowEndDevice attribute missing or non-boolean")
        end
        if expectBackend == "Backend: D3D12 (WARP)" and isLowEnd ~= true then
            return fail("Expected IsLowEndDevice == true on WARP software adapter")
        end

        if testSubsystems == "1" then
            -- 1. SaveTextureToPNG (including resize to 32x32)
            local tex = ParaAsset.LoadTexture("_ctest_tex", "Texture/whitedot.png", 1)
            if not tex or not tex:IsValid() then
                return fail("LoadTexture failed for Texture/whitedot.png")
            end
            if type(tex.SaveTextureToPNG) ~= "function" then
                return fail("ParaAssetObject.SaveTextureToPNG binding missing")
            end
            tex:SaveTextureToPNG("_ctest_saved_tex.png", 32, 32)

            -- 2. ParaAsset.ConvertGLB (Assimp OBJ -> GLB2 export)
            if type(ParaAsset.ConvertGLB) ~= "function" then
                return fail("ParaAsset.ConvertGLB binding missing")
            end
            ParaAsset.ConvertGLB("_ctest_tri.obj,0 0 0 0 0 0 1,_ctest_tri.glb")

            -- 3. Transparent script Fancy shader interception -> embedded unified runtime shader
            local fxMrt = ParaAsset.LoadEffectFile("_ctest_mrt_blocks", "script/apps/Aries/Creator/Game/Shaders/mrt_blocks.fx")
            if not fxMrt then
                return fail("LoadEffectFile returned nil for mrt_blocks.fx")
            end
            fxMrt:LoadAsset()
            if not fxMrt:IsValid() then
                return fail("Transparent script shader interception failed for mrt_blocks.fx")
            end
            local fxComp = ParaAsset.LoadEffectFile("_ctest_composite", "script/apps/Aries/Creator/Game/Shaders/composite.fx")
            if not fxComp then
                return fail("LoadEffectFile returned nil for composite.fx")
            end
            fxComp:LoadAsset()
            if not fxComp:IsValid() then
                return fail("Transparent script shader interception failed for composite.fx")
            end
        end

        local f = ParaIO.open(outFile, "w")
        if f:IsValid() then
            f:WriteString("PASS\n" .. tostring(stats) .. "\n")
            f:close()
        end
        ParaGlobal.Exit(0)
    elseif tick > 60 then
        return fail("Timed out in CTest Lua loop")
    end
end

local function safe_main_loop()
    local ok, err = pcall(main_loop)
    if not ok then
        fail("Lua exception: " .. tostring(err))
    end
end

NPL.this(safe_main_loop)
'@

$backends = @(
    @{ Tag = "d3d11";      Renderer = "d3d11";  Extra = "";             Expect = "Backend: D3D11";        Subsys = "0" },
    @{ Tag = "d3d12";      Renderer = "d3d12";  Extra = "";             Expect = "Backend: D3D12";        Subsys = "1" },
    @{ Tag = "d3d12_warp"; Renderer = "d3d12";  Extra = "--d3d12-warp"; Expect = "Backend: D3D12 (WARP)"; Subsys = "0" },
    @{ Tag = "vulkan";     Renderer = "vulkan"; Extra = "";             Expect = "Backend: Vulkan";       Subsys = "0" },
    @{ Tag = "opengl";     Renderer = "opengl"; Extra = "";             Expect = "Backend: OpenGL";       Subsys = "0" },
    @{ Tag = "webgpu_fb";  Renderer = "webgpu"; Extra = "";             Expect = "Backend: D3D12";        Subsys = "0" }
)

$texDir = Join-Path $BinDir "Texture"
$createdTexDir = -not (Test-Path $texDir)
$whiteDotPath = Join-Path $texDir "whitedot.png"
$createdWhiteDot = -not (Test-Path $whiteDotPath)

$staged = @(
    "_ctest_harness.lua",
    "_ctest_saved_tex.png",
    "_ctest_tri.obj",
    "_ctest_tri.glb"
)
foreach ($b in $backends) {
    $t = $b.Tag
    $staged += @("_ctest_$t.txt", "_ctest_$t.bmp")
}

try {
    if ($createdTexDir) { New-Item -ItemType Directory -Path $texDir -Force | Out-Null }
    if ($createdWhiteDot) {
        $wb = [System.Drawing.Bitmap]::new(8, 8, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        for ($wy = 0; $wy -lt 8; $wy++) {
            for ($wx = 0; $wx -lt 8; $wx++) {
                $wb.SetPixel($wx, $wy, [System.Drawing.Color]::FromArgb(255, 255, 255, 255))
            }
        }
        $wb.Save($whiteDotPath, [System.Drawing.Imaging.ImageFormat]::Png)
        $wb.Dispose()
    }

    foreach ($f in $staged) {
        $p = Join-Path $BinDir $f
        if (Test-Path $p) { Remove-Item $p -Force -ErrorAction SilentlyContinue }
    }

    # Minimal Wavefront OBJ triangle for ParaAsset.ConvertGLB test
    $objContent = @"
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
vn 0.0 0.0 1.0
vt 0.0 0.0
vt 1.0 0.0
vt 0.0 1.0
f 1/1/1 2/2/1 3/3/1
"@
    [System.IO.File]::WriteAllText((Join-Path $BinDir "_ctest_tri.obj"), $objContent, [System.Text.Encoding]::ASCII)
    [System.IO.File]::WriteAllText((Join-Path $BinDir "_ctest_harness.lua"), $luaHarness, (New-Object System.Text.UTF8Encoding($false)))

    foreach ($b in $backends) {
        $tag = $b.Tag
        $outName = "_ctest_$tag.txt"
        $shotName = "_ctest_$tag.bmp"
        $outFull = Join-Path $BinDir $outName
        $shotFull = Join-Path $BinDir $shotName

        $argList = @(
            "bootstrapper=`"_ctest_harness.lua`"",
            "--renderer=$($b.Renderer)",
            "width=640",
            "height=480",
            "windowed=true",
            "out=`"$outName`"",
            "shot=`"$shotName`"",
            "expect_backend=`"$($b.Expect)`"",
            "test_subsystems=`"$($b.Subsys)`""
        )
        if ($b.Extra -ne "") {
            $argList += $b.Extra
        }

        $proc = Start-Process -FilePath $clientExe -WorkingDirectory $BinDir -ArgumentList $argList -PassThru
        if (-not $proc.WaitForExit(20000)) {
            $proc.Kill()
            throw "[$tag] Timed out after 20s"
        }
        if ($proc.ExitCode -ne 0) {
            $errTxt = if (Test-Path $outFull) { Get-Content $outFull -Raw } else { "(no output file)" }
            throw "[$tag] Exited with code $($proc.ExitCode): $errTxt"
        }
        if (-not (Test-Path $outFull)) {
            throw "[$tag] Missing output file $outFull"
        }
        $resText = Get-Content $outFull -Raw
        if (-not $resText.StartsWith("PASS")) {
            throw "[$tag] Test failed: $resText"
        }
        if (-not (Test-Path $shotFull)) {
            throw "[$tag] Missing screenshot $shotFull"
        }
        Write-Host "PASS [$tag]: $($b.Expect)"
    }

    # Verify visual PSNR parity against D3D11 baseline
    $refBmp = Join-Path $BinDir "_ctest_d3d11.bmp"
    foreach ($cmpTag in @("d3d12", "d3d12_warp", "vulkan", "opengl", "webgpu_fb")) {
        $cmpBmp = Join-Path $BinDir "_ctest_$cmpTag.bmp"
        $psnr = Compute-PSNR -PathA $refBmp -PathB $cmpBmp
        Write-Host "PSNR [d3d11 vs $cmpTag] = $psnr dB"
        if ($psnr -lt 35.0) {
            throw "PSNR too low between d3d11 and $cmpTag : $psnr dB (< 35.0 dB)"
        }
    }

    # Verify SaveTextureToPNG output (32x32 PNG)
    $pngPath = Join-Path $BinDir "_ctest_saved_tex.png"
    if (-not (Test-Path $pngPath)) {
        throw "SaveTextureToPNG did not create $pngPath"
    }
    $pngBytes = [System.IO.File]::ReadAllBytes($pngPath)
    if ($pngBytes.Length -lt 32 -or $pngBytes[0] -ne 0x89 -or $pngBytes[1] -ne 0x50 -or $pngBytes[2] -ne 0x4E -or $pngBytes[3] -ne 0x47) {
        throw "SaveTextureToPNG produced invalid PNG signature"
    }
    $savedImg = [System.Drawing.Bitmap]::new($pngPath)
    try {
        if ($savedImg.Width -ne 32 -or $savedImg.Height -ne 32) {
            throw "SaveTextureToPNG resize mismatch: expected 32x32, got $($savedImg.Width)x$($savedImg.Height)"
        }
        $px = $savedImg.GetPixel(16, 16)
        if ($px.R -lt 240 -or $px.G -lt 240 -or $px.B -lt 240) {
            throw "SaveTextureToPNG pixel mismatch at (16,16): R=$($px.R) G=$($px.G) B=$($px.B)"
        }
    } finally {
        $savedImg.Dispose()
    }
    Write-Host "PASS [SaveTextureToPNG]: Valid 32x32 PNG ($($pngBytes.Length) bytes)"

    # Verify ConvertGLB output (valid binary glTF 2.0 'glTF' header)
    $glbPath = Join-Path $BinDir "_ctest_tri.glb"
    if (-not (Test-Path $glbPath)) {
        throw "ParaAsset.ConvertGLB did not create $glbPath"
    }
    $glbBytes = [System.IO.File]::ReadAllBytes($glbPath)
    if ($glbBytes.Length -lt 20 -or $glbBytes[0] -ne 0x67 -or $glbBytes[1] -ne 0x6C -or $glbBytes[2] -ne 0x54 -or $glbBytes[3] -ne 0x46) {
        throw "ParaAsset.ConvertGLB output missing 'glTF' magic header"
    }
    $glbVersion = [System.BitConverter]::ToUInt32($glbBytes, 4)
    if ($glbVersion -ne 2) {
        throw "ParaAsset.ConvertGLB unexpected glTF version: $glbVersion"
    }
    Write-Host "PASS [ConvertGLB]: Valid glTF 2.0 binary ($($glbBytes.Length) bytes)"
} finally {
    foreach ($f in $staged) {
        $p = Join-Path $BinDir $f
        if (Test-Path $p) { Remove-Item $p -Force -ErrorAction SilentlyContinue }
    }
    if ($createdWhiteDot -and (Test-Path $whiteDotPath)) {
        Remove-Item $whiteDotPath -Force -ErrorAction SilentlyContinue
    }
    if ($createdTexDir -and (Test-Path $texDir)) {
        Remove-Item $texDir -Recurse -Force -ErrorAction SilentlyContinue
    }
}
