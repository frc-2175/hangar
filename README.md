# hangar

A tool for experimenting with robot climbs and hangs.

Written in C, using [raylib](https://www.raylib.com/). Built for the web using WebAssembly.

## Building

This project pretty much follows the [standard raylib WASM instructions](https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)). As those instructions indicate, you will need Emscripten. Follow the instructions in step 1 of the Raylib web instructions. **After installing and activating Emscripten, restart your computer.** Some environment variables are extremely stubborn.

You should not need to compile raylib itself (`libraylib.a`) as long as you have Emscripten.

You must `cd` into the `src` folder to build. The project can be build simply by running `make -e`. On Windows you may need to install [MinGW](https://osdn.net/projects/mingw/), and then run `mingw32-make -e`.

## Running

In another terminal, cd into `src` and run `python -m http.server`. This will launch a web server on port 8000 or 8080. Visit http://localhost:8080/hangar.html to view the app (changing the port number if necessary).
