$ErrorActionPreference = "Stop"

$cmake = "C:\Program Files\CMake\bin\cmake.exe"
$gccBin = "C:\Program Files (x86)\Embarcadero\Dev-Cpp\TDM-GCC-64\bin"
$buildDir = "build_mingw"

$env:PATH = "$gccBin;$env:PATH"

if (-not (Test-Path $cmake)) {
    throw "CMake tidak ditemukan di $cmake"
}

if (-not (Test-Path $buildDir)) {
    & $cmake -S . -B $buildDir -G "MinGW Makefiles" `
        -DCMAKE_MAKE_PROGRAM="$gccBin\mingw32-make.exe" `
        -DCMAKE_C_COMPILER="$gccBin\gcc.exe" `
        -DCMAKE_CXX_COMPILER="$gccBin\g++.exe"
}

& $cmake --build $buildDir --config Release -j 4

$exe = Join-Path (Resolve-Path "$buildDir\bin").Path "traffic_microsim.exe"
& $exe
