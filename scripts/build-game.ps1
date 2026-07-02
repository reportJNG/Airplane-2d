param(
  [string]$RaylibVersion = "6.0"
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$gameRoot = Join-Path $repoRoot "AirPlane Game"
$outputRoot = Join-Path $repoRoot "public\game"
$cacheRoot = Join-Path $env:TEMP "airplane-raylib-web"
$raylibZip = Join-Path $cacheRoot "raylib-$RaylibVersion`_webassembly.zip"
$raylibRoot = Join-Path $cacheRoot "raylib-$RaylibVersion`_webassembly"
$raylibUrl = "https://github.com/raysan5/raylib/releases/download/$RaylibVersion/raylib-$RaylibVersion`_webassembly.zip"

function Use-Emscripten {
  if (Get-Command emcc -ErrorAction SilentlyContinue) {
    return
  }

  $candidate = "C:\Work\Tools\emsdk\emsdk_env.bat"
  if (Test-Path $candidate) {
    $env:EMSDK_QUIET = "1"
    cmd /c "`"$candidate`" >nul && set" | ForEach-Object {
      $line = $_
      $index = $line.IndexOf("=")
      if ($index -gt 0) {
        $name = $line.Substring(0, $index)
        $value = $line.Substring($index + 1)
        Set-Item -Path "Env:$name" -Value $value
      }
    }
  }

  if (-not (Get-Command emcc -ErrorAction SilentlyContinue)) {
    throw "Emscripten emcc was not found. Install/activate emsdk, then rerun npm run game:build."
  }
}

New-Item -ItemType Directory -Force -Path $cacheRoot, $outputRoot | Out-Null

if (-not (Test-Path $raylibZip)) {
  Write-Host "Downloading raylib $RaylibVersion WebAssembly package..."
  Invoke-WebRequest -Uri $raylibUrl -OutFile $raylibZip
}

if (-not (Test-Path $raylibRoot)) {
  $extractRoot = Join-Path $cacheRoot "extract"
  if (Test-Path $extractRoot) {
    Remove-Item -Recurse -Force $extractRoot
  }
  Expand-Archive -Path $raylibZip -DestinationPath $extractRoot
  Move-Item -Path (Join-Path $extractRoot "raylib-$RaylibVersion`_webassembly") -Destination $raylibRoot
  Remove-Item -Recurse -Force $extractRoot
}

Use-Emscripten

Get-ChildItem -Path $outputRoot -Filter "airplane.*" | Remove-Item -Force

$sources = @(
  (Join-Path $gameRoot "game.c"),
  (Join-Path $gameRoot "headers\player.c"),
  (Join-Path $gameRoot "headers\enemy.c"),
  (Join-Path $gameRoot "headers\boss.c")
)

$imagesPreload = "$(Join-Path $gameRoot "images")@images"
$soundsPreload = "$(Join-Path $gameRoot "sounds")@sounds"
$iconPreload = "$(Join-Path $gameRoot "icon")@icon"

$arguments = @(
  $sources,
  "-o", (Join-Path $outputRoot "airplane.js"),
  "-I", (Join-Path $raylibRoot "include"),
  (Join-Path $raylibRoot "lib\libraylib.web.a"),
  "-DPLATFORM_WEB",
  "-Os",
  "-Wall",
  "-s", "USE_GLFW=3",
  "-s", "ASSERTIONS=1",
  "-s", "INITIAL_MEMORY=134217728",
  "-s", "EXPORTED_FUNCTIONS=['_main','_SetBrowserKey']",
  "-s", "EXPORTED_RUNTIME_METHODS=['ccall']",
  "--preload-file", $imagesPreload,
  "--preload-file", $soundsPreload,
  "--preload-file", $iconPreload
)

Write-Host "Building Airplane Game WebAssembly..."
& emcc @arguments

if ($LASTEXITCODE -ne 0) {
  throw "emcc failed with exit code $LASTEXITCODE"
}

Write-Host "Wrote public/game/airplane.js, airplane.wasm, and airplane.data"
