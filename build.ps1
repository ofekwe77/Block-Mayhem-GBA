# BLOCK MAYHEM build entry point for Windows PowerShell.
#
#   .\build.ps1                 -> generate assets + build BlockMayhem.gba
#   .\build.ps1 rom             -> build the ROM only (assets must exist)
#   .\build.ps1 assets          -> regenerate graphics/audio from tools/*.py
#   .\build.ps1 tests           -> build + run host-side unit tests and simulations
#   .\build.ps1 harness         -> build the headless mGBA test harness
#   .\build.ps1 emu <script>    -> run an emulator scenario (tools/harness)
#   .\build.ps1 verify          -> full release verification (verify_release)
#   .\build.ps1 clean           -> remove build output
#
# GNU make / gcc cannot cope with the spaces and non-ASCII characters in this
# project's path, so the project folder is mapped to an ASCII drive letter with
# `subst` (session-only, no admin rights, no global settings changed).
#
# Environment overrides:
#   BM_MSYS2  MSYS2 root containing devkitPro packages (default C:\msys64)
#   BM_DRIVE  Drive letter used for the ASCII mapping   (default M:)
param(
    [Parameter(Position = 0)][string]$Command = "all",
    [Parameter(Position = 1, ValueFromRemainingArguments = $true)][string[]]$Rest
)

$ErrorActionPreference = "Stop"
$msys = if ($env:BM_MSYS2) { $env:BM_MSYS2 } else { "C:\msys64" }
$drive = if ($env:BM_DRIVE) { $env:BM_DRIVE } else { "M:" }
$root = $PSScriptRoot

$bash = Join-Path $msys "usr\bin\bash.exe"
if (-not (Test-Path $bash)) {
    Write-Error "MSYS2 not found at $msys. See README.md (Installing dependencies)."
    exit 2
}

# Map the project to an ASCII-only drive letter if it is not mapped already.
$mapped = $false
foreach ($line in (subst)) {
    if ($line -match "^$([regex]::Escape($drive))\\: => (.*)$") {
        if ($Matches[1].TrimEnd('\') -ieq $root.TrimEnd('\')) { $mapped = $true }
        else { Write-Error "$drive is already mapped to $($Matches[1]). Set BM_DRIVE to a free letter."; exit 2 }
    }
}
if (-not $mapped) {
    if (Test-Path "$drive\") { Write-Error "$drive is in use. Set BM_DRIVE to a free letter."; exit 2 }
    subst $drive $root
}

$letter = $drive.Substring(0, 1).ToLower()
$env:MSYSTEM = "MSYS"
$env:CHERE_INVOKING = "1"
$argString = ($Rest | ForEach-Object { "'" + ($_ -replace "'", "'\''") + "'" }) -join " "
& $bash -lc "cd /$letter && ./tools/bm.sh $Command $argString"
exit $LASTEXITCODE
