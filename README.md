# GBALink
Linking Library for GameCube to GBA (wip)

This project aims to be an easy-to-use library / framework so people can use the GBA with their GameCube homebrew without extensive knowledge of GBA programming.

## Four levels of abstraction:
- GBALinkJoyBoot is the GBA multiboot, and it is preferrable for the programmer to use a binary version of this in the GC filesystem. Included here for easy testing. It implements the GBALink API on the GBA side. I still need to upload the source for this..
- GBATransfer is the low-level send & receive packets, and the multiboot sending code.
- GBALink is the message passing and general purpose packet commands in a cleaner way.
- AGBPAD is a high-level lib for reading the GBA like it's a controller, with rumble, tilt, gyro, and light sensors in fancy structs and button polling.

## Current issues:
- No threading. Say goodbye to your framerate..
- No use of stacks or queues. Messages might get mixed up if many different ones are used..
- Poor error handling. When GBA & GameCube become desynced it's a gamble if they will find each other again..
- Joy Boot Program is not compressed. 

## Feature list:
- Low- and high-level GBA controller interpretation with rumble, tilt, gyro, and ambient light sensors. (🔧)
- Full-size images from the GameCube. (🔧)
- Streaming video from the GameCube such as an additional framebuffer. Targeting 5FPS full-res, 10-20FPS full-res compressed, 20FPS half-res, or 30-60FPS half-res compressed. Full-res: 240x160; half-res: 120x80. (❌)
- "Streaming" audio from the GameCube, Wii Remote style (cache to GBA and playback on command). (❌)
- Displaying the "Look at TV" graphic. (❌)
- Tilemaps from the GameCube. (❌)
- Tilemaps converted from full-size images. (❌)
- Sprites and sprite manipulation with lerp. (❌)

## Contributing:
Have a change you'd like to make? Feel free to put in a PR!
It would be highly preferred if the change you made was tested / designed for a specific implementation (i.e. [Octave](https://github.com/mholtkamp/octave) or [VBAGX](https://github.com/dborth/vbagx/)).
