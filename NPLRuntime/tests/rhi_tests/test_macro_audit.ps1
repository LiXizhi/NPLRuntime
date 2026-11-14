param(
    [Parameter(Mandatory = $true)]
    [string]$ParaEngineDir
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $ParaEngineDir)) {
    throw "ParaEngine directory not found: $ParaEngineDir"
}

$files = Get-ChildItem -Path $ParaEngineDir -Recurse -File -Include *.h, *.hpp, *.cpp, *.c, *.mm
$hits = $files | Select-String -Pattern "\bUSE_DIRECTX_RENDERER\b|\bUSE_OPENGL_RENDERER\b"

if ($hits) {
    Write-Error ("Found legacy compile-time renderer macros in ParaEngine (" + $hits.Count + " occurrences):")
    foreach ($h in $hits) {
        Write-Host ("  " + $h.Path + ":" + $h.LineNumber + ": " + $h.Line.Trim())
    }
    exit 1
}

Write-Host ("PASS: Zero USE_DIRECTX_RENDERER / USE_OPENGL_RENDERER occurrences across " + $files.Count + " ParaEngine source files.")
exit 0
