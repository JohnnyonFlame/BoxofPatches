# **Box of Patches**:
A repository for unofficial game patches.

## **BOX86 Targets**:
---
### Usage: 
The box86-focused patches requires you to set the env var `BOX86_LD_PRELOAD=libTargetName.so` (e.g.: `libShovelKnight.so`) so they are loaded into their respective processes.

### Targets:
 Target                          | Features                                 | Aditional Usage Info
---------------------------------|------------------------------------------|--------------------------------
 [Iconoclasts](Iconoclasts/)     | Workaround for persistent slowdowns.     | Set env var `CHOWDREN_FPS=fps`
 [FreedomPlanet](FreedomPlanet/) | Workaround for persistent slowdowns.     | n/a
 [ShovelKnight](ShovelKnight/)   | Removes the "STEAM IS NOT RUNNING." nag. | n/a

## **Acknowledgements**:
---
* Thanks [xblyak](https://github.com/herumi/xbyak), for the code emitters.
* Thanks [box86](https://github.com/ptitSeb/box86/), for the amazing Linux Userspace emulator. 

## **License**:
---
This is *free software*. The source files in this repository are released under the [Modified BSD License](LICENSE.md), see the license file for more information.