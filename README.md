<div align="center">

# cubing

**A Rubik's cube solving timer tool**

![C](https://img.shields.io/badge/C-C99-00599C?logo=c&logoColor=white)
![raylib](https://img.shields.io/badge/raylib-6.2-black?logo=raylib&logoColor=white)
![Make](https://img.shields.io/badge/build-Make-A42E2B?logo=gnu&logoColor=white)
![Web](https://img.shields.io/badge/web-WebAssembly-654FF0?logo=webassembly&logoColor=white)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Web-lightgrey)
![Status](https://img.shields.io/badge/status-in%20development-orange)
![License](https://img.shields.io/badge/license-zlib-blue)

</div>

- Timer can be hidden to avoid stressing over time.

- Timing can be paused and resumed for solves that require more than one measure.

- A random scramble can be calculated although the current approach of random generation is insufficient for covering the entirety of the state space of the cube. 
    
    An estimate of 40 to 80 moves are apparently needed to correctly represent the space (didn't remember to write down the sources, sorry) so the current 20 moves that ship as default **are not enough** to correctly generate truly random scrambles.

    The correct approach would be to generate a random state of the cube, to solve it using something like Kociemba’s two-phase algorithm and then to invert the solve into a scramble. 
    
    This method was not used due to lack of public implementations of the algorithm in C. This might change in the future if I end up bored enough as to implement it.


## Build

The project uses Make to build and is currently configured with the following options:
| Command | Use case |
|---|---|
| make run | To compile and run for desktop. Right now it's only configured for Linux although I might expand to other platforms in the future |
| make debug | To compile with debugger friendly flags (no optimizations) |
| make web | To compile to WASM. Needs the [Emscripten](https://emscripten.org/) compiler and an installation of [Raylib](https://www.raylib.com/) built for WASM (path must be provided in the .env file) |
| make serve | To serve the WASM build in a local server using python |


## Status

In development. Live [here](https://bentor.dev/cubing).

## License


This project is licensed under the [zlib License](LICENSE).

It is based on the raylib [shapes] example "digital clock" by Hamza Rahal
([@hmz-rhl](https://github.com/hmz-rhl)) and Ramon Santamaria
([@raysan5](https://github.com/raysan5)), also under the zlib license.
It uses [raylib](https://www.raylib.com/), which is licensed under the same terms.