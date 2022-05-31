pwsh -c if(!(Test-Path "build")){New-Item "build" -ItemType Directory}

clang -O3 -std=c17 -Iexternal/nuklear/include -lGdi32.lib -lUser32.lib -lMsimg32.lib external/nuklear/src/*.c src/*.c src/gui/*.c src/core/*.c src/app/*.c -o build/MEdit.exe