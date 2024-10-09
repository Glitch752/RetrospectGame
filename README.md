# Untitled DOS rasterizer for Retrospect

This is a demo of 3D rasterization using fixed-point math with DOS 13h mode.
There are some things currently messed up, but I just wanted to get something submitted before the deadline.
Sorry that it's pretty broken in places. Notably, there's no near plane clipping and the depth buffer is pretty wrong.
I implemented a physics simulation, but I couldn't get it to work properly yet. I'll try to update this when I can.

## Controls:

- Move: WASD / space / left shift
- Pan camera: Q/E, pitch camera: Z/X
- Toggle controls hint visibility: J
- Show controls menu: H
- Exit: ESC

Thanks for playing even if it's broken!

## Development
I have a Justfile set up to compile and run the game. It assumnes `dosbox-x.exe` is in your PATH, but it should be easily adapted to Linux or other environments.
```
just run
```