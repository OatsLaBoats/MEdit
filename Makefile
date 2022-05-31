CC = clang
CFLAGS = -O0 -g -std=c17 -Wall -Wpedantic -Wextra -Wno-unused-command-line-argument -Wno-gnu-zero-variadic-macro-arguments
OUTPUT = MEdit

CONSTANTS = 
INCLUDES = -Iexternal/nuklear/include
LIBS = -lGdi32.lib -lUser32.lib -lMsimg32.lib

OBJECTS = build/nuklear.o \
		  build/main.o \
		  build/main_window_proc.o \
		  build/font.o \
		  build/gui.o \
		  build/renderer.o \
		  build/window.o \
		  build/array.o \
		  build/memscan.o \
		  build/os.o \
		  build/process_utils.o \
		  build/text.o \
		  build/thread.o \
		  build/timer.o \
		  build/app_utils.o \
		  build/app.o \
		  build/scanner.o \
		  build/editor.o \
		  build/select_process.o \

COMPILE = $(CC) $(CFLAGS) $(CONSTANTS) $(INCLUDES) $(LIBS)
COMPILE_OBJECT = $(COMPILE) -c -o $@ $<

touch = pwsh -c (Get-Item $(1)).LastWriteTime=Get-Date
UPDATE_FILE = @$(call touch,$@)

EXE_OUTPUT = build/$(OUTPUT).exe

all: build $(EXE_OUTPUT)

# checks if build directory exists and creates it if needed
build:
	@pwsh -c if(!(Test-Path "build")){New-Item "build" -ItemType Directory}

$(EXE_OUTPUT): $(OBJECTS)
	$(COMPILE) -o $@ $^

build/nuklear.o: external/nuklear/src/nuklear.c external/nuklear/src/nuklear.h
	$(COMPILE) -D_CRT_SECURE_NO_WARNINGS -Wno-unused-function -c -o $@ $<

build/main.o: src/main.c src/core/utils.h src/core/os.h src/gui/gui.h src/app/app.h src/window_procs.h
	$(COMPILE_OBJECT)

build/main_window_proc.o: src/main_window_proc.c src/window_procs.h
	$(COMPILE_OBJECT)

build/font.o: src/gui/font.c src/gui/font.h src/core/utils.h
	$(COMPILE_OBJECT)

build/gui.o: src/gui/gui.c src/gui/gui.h src/core/utils.h src/core/os.h
	$(COMPILE_OBJECT)

build/renderer.o: src/gui/renderer.c src/gui/renderer.h src/core/utils.h
	$(COMPILE_OBJECT)

build/window.o: src/gui/window.c src/gui/window.h src/core/utils.h
	$(COMPILE_OBJECT)

build/array.o: src/core/array.c src/core/array.h src/core/utils.h
	$(COMPILE_OBJECT)

build/memscan.o: src/core/memscan.c src/core/memscan.h src/core/utils.h src/core/thread.h
	$(COMPILE_OBJECT)

build/os.o: src/core/os.c src/core/os.h src/core/utils.h
	$(COMPILE_OBJECT)

build/process_utils.o: src/core/process_utils.c src/core/process_utils.h src/core/utils.h
	$(COMPILE_OBJECT)

build/text.o: src/core/text.c src/core/text.h src/core/utils.h
	$(COMPILE_OBJECT)

build/thread.o: src/core/thread.c src/core/thread.h src/core/utils.h
	$(COMPILE_OBJECT)

build/timer.o: src/core/timer.c src/core/timer.h src/core/utils.h
	$(COMPILE_OBJECT)

build/app_utils.o: src/app/app_utils.c src/app/app_utils.h src/core/utils.h
	$(COMPILE_OBJECT)

build/app.o: src/app/app.c src/app/app.h src/core/utils.h src/app/select_process.h src/app/scanner.h src/app/editor.h
	$(COMPILE_OBJECT)

build/scanner.o: src/app/scanner.c src/app/scanner.h src/core/utils.h src/core/thread.h src/app/app_utils.h
	$(COMPILE_OBJECT)

build/editor.o: src/app/editor.c src/app/editor.h
	$(COMPILE_OBJECT)

build/select_process.o: src/app/select_process.c src/app/select_process.h src/core/utils.h
	$(COMPILE_OBJECT)

src/gui/font.h: src/core/utils.h
	$(UPDATE_FILE)

src/gui/gui.h: src/gui/window.h src/gui/renderer.h src/gui/font.h
	$(UPDATE_FILE)

src/gui/renderer.h: src/gui/window.h src/gui/font.h
	$(UPDATE_FILE)

src/core/memscan.h: src/core/array.h src/core/process_utils.h src/core/text.h
	$(UPDATE_FILE)

src/core/os.h: src/core/text.h
	$(UPDATE_FILE)

src/app/app_utils.h: src/app/app.h
	$(UPDATE_FILE)

src/app/app.h: src/core/process_utils.h src/core/array.h src/core/text.h src/core/memscan.h src/core/timer.h
	$(UPDATE_FILE)

src/app/direct_editor.h: src/app/app.h
	$(UPDATE_FILE)

src/app/scanner.h: src/app/app.h
	$(UPDATE_FILE)

src/app/editor.h: src/app/app.h src/app/app_utils.h
	$(UPDATE_FILE)

src/app/select_process.h: src/app/app.h
	$(UPDATE_FILE)

src/app/watch.h: src/app/app.h
	$(UPDATE_FILE)

.PHONY: clean
clean:
	@pwsh -c Remove-Item ./build/*

.PHONY: objclean
objclean:
	@pwsh -c Remove-Item ./build/*.o

.PHONY: run
run: all
	@build/$(OUTPUT)
