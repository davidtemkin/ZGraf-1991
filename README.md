# ZGraf / Manic Episode (1991)

Original source code for **Manic Episode**, a stereoscopic 3D tunnel shooter for classic Macintosh.

![Screenshot](Screenshots/Screenshot%20(Color).png)

## About

Manic Episode is Mac 3D arcade game from 1991 featuring:

- Real-time stereoscopic 3D (red/green anaglyph glasses)
- Tunnel-based gameplay with enemies, power-ups, and obstacles
- Custom object-oriented game engine ("ZGraf") written in Think C
- Custom renderer written in 68000 assembly language
- Support for both color and black & white Macs

The game runs in a tunnel viewed from first-person perspective. The player navigates through the tunnel, shooting enemies (Crosses, Saucers, Aphids) while avoiding obstacles (Blockers) and collecting power-ups (Things).

The game was on display as part of the Boston Computer Museum's 1992 exhibit "Tools and Toys: The Amazing Personal Computer".

## Technical Details

- **Language**: Think C with 68000 assembly for rendering
- **Target**: Macintosh System 6/7
- **Display**: Runs on everything from the original Macintosh (1984) through the color Macs shipped in the mid-1990s, prior to the PowerPC transition
- **3D Engine**: Custom fixed-point math, perspective projection
- **Color**: Optimized for 2-bit color mode, using color lookup tables for left/right eye image blending in stereo mode

The codebase uses a custom object-oriented pattern implemented in C, with inheritance tables and virtual method dispatch.

## Files

- `*.c` / `*.h` - Source code
- `Sounds/` - Sound resources
- `Screenshots/` - Original game screenshots
- `ZGraf.π` - Think C project file (resource fork)

## Building

This code was written for Think C on classic Macintosh. To build today, you would need:

- A classic Mac emulator (Mini vMac, Basilisk II, SheepShaver)
- Think C 5.0 or later
- System 6 or 7

## See Also

- [ZGraf-2025](https://github.com/davidtemkin/ZGraf-2025) - Modern web port of this game

## License

MIT License - Copyright (c) 1991-2025 David Temkin
