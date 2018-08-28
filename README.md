<h1 align="center">
    <img src="https://i.imgur.com/YoQdq7Y.png" alt="CHIP-8" height="200" width="400">
    <br>
    CHIP-8
</h1>

<h2 align="center">
    Cross-platform Chip-8 interpreter written in C and OpenGL.
</h2>

<p align="center">
    <a href="#key-features">Key Features</a> •
    <a href="#installation">Installation</a> •
    <a href="#how-to-use">How To Use</a> •
    <a href="#credits">Credits</a> •
    <a href="#license">License</a> •
</p>

<h1 align="center">
    <img src="https://i.imgur.com/6sksDU8.gif" alt="pong gif" height="600" width="1200">
</h1>

## Key Features
* Cross-platform - utilizes GLFW to handle native window management
    - Windows
    - Linux
    - Mac
* Adjustable frame rate
* Custom resolution
* Modifiable color scheme
<hr>

## Installation
* Required modules - to clone and build this aplication the following must be installed:
    - [OpenGL V4.0+](https://www.opengl.org/)
    - [Git](https://git-scm.com/)
    - [CMake](https://cmake.org/)
* Building - From the command line run the following commands:
```
# Clone the repository, and enter the directory
git clone https://github.com/grez96/CHIP-8.git
cd CHIP-8

# Create the build directory (out of source build) and enter it
mkdir build
cd build

# Run CMake build system
cmake ..
```
* Compilation - platform dependent
    - Linux and Mac systems (Windows as well if MiniGW is installed) can simply run make to create an executable
    - Windows systems will have to open the .sln file produced by CMake with Visual Studios and compile/run from there
<hr>

## How To Use
* Main-menu - Command Line Interface
    - ROMs are numbered from 1-7, simply specify the number that corresponds to the desired ROM and press enter
    - 0 to exit
* CHIP-8 - ROM Interpreter
    - ESC to exit any time
    - Windowing features such as minimizing, maximizing, closing, and resizing work in their native expected way
    - default keybindings:
```
+-+-+-+-+	+-+-+-+-+
|1|2|3|C|	|1|2|3|4|
+-+-+-+-+	+-+-+-+-+
|4|5|6|D|	|Q|W|E|R|
+-+-+-+-+	+-+-+-+-+
|7|8|9|E|	|A|S|D|F|
+-+-+-+-+	+-+-+-+-+
|A|0|B|F|	|Z|X|C|V|
+-+-+-+-+	+-+-+-+-+
```
<hr>

## Credits
* Developed and maintained by Gleb Reznicov
* Mac OSX debugging done by Xin Song Felix Zhang
* The following open source software/packages were used:
    - [OpenGL](https://www.opengl.org/)
    - [GLFW](https://www.glfw.org/)
    - [GLAD](https://glad.dav1d.de/)
    - [math_3d](https://github.com/arkanis/single-header-file-c-libs/blob/master/math_3d.h)
* Chip-8 documentation:
    - [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
    - [CHIP-8 - Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)
* ROMs provided by [dmatlack's](https://github.com/dmatlack/chip8/tree/master/roms) ROM compilation
<hr>

## License
MIT
