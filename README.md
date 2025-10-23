# Simple platformer game prototype, written in C with Raylib
I treat this project as mostly a learning experience, due to wanting to sink my teeth into C for a while now.
Raylib turned out to be a fantastic library which let me experience C programming while keeping it fun.

## This "game" includes:
- Animations!
- Simple physics and collisions (although a bit broken)!
- Definitely no spaghetti code!
- Shaders!
- A function to draw the also broken hitboxes!
- And possibly more in the future..!

## How to compile this?
For Windows:
`x86_64-w64-mingw32-gcc src/main.c -o main.exe -I include/ -L /linkyourownraylib/lib/imtoolazy -lraylib -lwinmm -lgdi32`

For Linux:
`idk just use gcc and figure it out i work on linux but compile this for windows`

