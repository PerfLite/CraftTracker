$targets = @(
    "C:\Games\Skyrim\[STB] Mod Organizer\mods\Craft Tracker",
    "C:\Games\RFAB\MO2\mods\Craft Tracker"
)

$srcDll = "C:\Dev\Skyrim mod prism ui\Crafting_tracker\build\windows\x64\release\CraftTracker.dll"
$srcHtml = "C:\Dev\Skyrim mod prism ui\Crafting_tracker\PrismaUI\views\CraftTracker\index.html"
$srcHammer = "C:\Dev\Skyrim mod prism ui\Crafting_tracker\PrismaUI\views\CraftTracker\hammer.png"

if (-not (Test-Path -LiteralPath $srcDll)) {
    Write-Error "DLL not found: $srcDll. Build first via 'xmake build -y'."
    exit 1
}

foreach ($target in $targets) {
    if (Test-Path -LiteralPath (Split-Path -Parent $target)) {
        New-Item -ItemType Directory -Force -Path "$target\SKSE\Plugins" | Out-Null
        New-Item -ItemType Directory -Force -Path "$target\PrismaUI\views\CraftTracker" | Out-Null

        [System.IO.File]::Copy($srcDll, "$target\SKSE\Plugins\CraftTracker.dll", $true)
        [System.IO.File]::Copy($srcHtml, "$target\PrismaUI\views\CraftTracker\index.html", $true)
        if (Test-Path -LiteralPath $srcHammer) {
            [System.IO.File]::Copy($srcHammer, "$target\PrismaUI\views\CraftTracker\hammer.png", $true)
        }

        Write-Host "[SUCCESS] Deployed to: $target" -ForegroundColor Green
    } else {
        Write-Host "[SKIP] Path parent does not exist: $target" -ForegroundColor Yellow
    }
}
