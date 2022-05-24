                     ████████
               ████████████████████
           ████████████████████████████
         ████████████████████████████████
       ██████    ████████████████    ██████
     ██████      ████████████████      ██████
     ████          ████████████          ████
   ████            ████████████            ████
   ████              ████████              ████
   ████              ████████              ████
 ████              ████    ████              ████
 ████              ██        ██              ████
 ████████████████████        ████████████████████
 ██████████████████████    ██████████████████████
   ████████████████████████████████████████████
   ██████████████████        ██████████████████
   ████████████████            ████████████████
     ██████████████            ██████████████
     ████████████                ████████████
       ██████████                ██████████
         ██████                    ██████
           ██████████        ██████████
               ████████████████████
                     ████████

                 x0r_jmp PRESENTS
   D U K E   N U K E M   F O R E V E R   2 0 0 1
                   ╔═══════════╗
 ╔═════════════════╣ GAME INFO ╠═════════════════╗
 ║                 ╚═══════════╝                 ║
 ║ Duke Nukem Forever brings back the king of    ║
 ║ action in the highly anticipated game set to  ║
 ║ pummel players with unprecedented             ║
 ║ interactivity, variety, realism, and Duke's   ║
 ║ special whoop-ass brand of humor. The first   ║
 ║ in-house Duke Nukem game by 3D Realms since   ║
 ║ Duke Nukem 3D.                                ║
 ║                                               ║
 ║ Powered by the latest in Unreal technology,   ║
 ║ Duke Nukem Forever delivers an intense First  ║
 ║ Person Shooter experience, with stunningly    ║
 ║ detailed characters in expansive interactive  ║
 ║ maps. Motion captured animation and realistic ║
 ║ area-sensitive damage take realism to scary   ║
 ║ new heights, and make picking off those alien ║
 ║ bastards all the more fun.                    ║
 ╚═══════════════════════════════════════════════╝
                 ╔═══════════════╗
 ╔═══════════════╣ RELEASE NOTES ╠═══════════════╗
 ║               ╚═══════════════╝               ║
 ║ We at x0r_jmp have spent a great deal of time ║
 ║ producing and procuring fixes to ensure this  ║
 ║ piece of gaming history can be preserved and  ║
 ║ enjoyed on modern systems.                    ║
 ║                                               ║
 ║ All of these fixes can be obtained from the   ║
 ║ 'Patches' directory included in the release.  ║
 ║                                               ║
 ║ Full source code is included, and can be      ║
 ║ compiled by following the steps that we have  ║
 ║ outlined in the 'BUILDING.txt' file.          ║
 ╚═══════════════════════════════════════════════╝
                   ╔═══════════╗
 ╔═════════════════╣ THANKS TO ╠═════════════════╗
 ║                 ╚═══════════╝                 ║
 ║ * The original 3D Realms for making the game. ║
 ║ * Everyone who made this possible.            ║
 ╚═══════════════════════════════════════════════╝

 YOU MAY NEVER KNOW US, BUT NEVER FORGET OUR GIFTS
                ALWAYS BET ON DUKE

# Welcome to Original Duke Nukem Forever 2001 Leak #

This version uses files leaked on 08/05/2002 at https://archive.org/details/duke-nukem-forever-2001-leak .

If you want to edit your windowed/fullscreen resolutions and settings, check inside Stable-Unstable/Players folder and edit the
DukeForever.ini file inside the folder of your Profile name. My monitor can't handle more than 1280x1024, but you will find those
values under Engine.WindowsClient list. You might also attempt to play with D3DDrv.D3DRenderDevice settings.

Supposedly offending resources or unauthorized images have been removed from the original leak, which were ColeTheme / WiederTheme.dtx and ColeTheme / WiederTheme.mds . Will be replaced at some point.

# Content #
- August 21
    System files only, install on top of October build.
    Some maps and features function differently.
    No source code available.

- October 26
    More stable and functional than August build.
    Full source code available.

- MegaPatch
    Patch to fix various bugs and restore nonfunctioning content.
    Combines all other patches where relevant.

- EAX Emulation
    Patch to enable surround sound through EAX emulation.

- KNI Assembly
    Patched assembly files to fix compilation issues.

- D3D8Wrapper
    Patch that uses WineD3D and ForceD3D9On12 to fix rendering issues.
    ForceD3D9On12 is used in DukeEd to avoid crashes caused by WineD3D.

- AugustRenderHack
    Patch to enable using the August build's Direct3D renderer with the October build.
    Partially fixes rendering of hair, at the cost of breaking various other features.

# Building #

COMPILING THE SOURCE CODE UNDER WINDOWS XP
1. Install Visual C++ 6.0 https://archive.org/details/microsoftvisualstudio60 (product code 110-1234567)
2. Install Visual C++ 6.0 SP5 and then the Processor Pack https://archive.org/details/vcpp5
3. Go to System Properties -> Advanced -> Environment Variables and set the following:
BUILD_ROOT_DUKE = path to game files
C32BIN = C:\Program Files\Microsoft Visual Studio\VC98\Bin
C32RC = C:\Program Files\Microsoft Visual Studio\Common\MSDev98\Bin
MASM_BIN = C:\Program Files\Microsoft Visual Studio\VC98\Bin
4. Copy the contents of Patches\KNI Assembly\ into the root directory.
5. Open Duke4.dsw and repeatedly choose cancel followed by no until no more prompts appear.
6. Go to Tools -> Options -> Directories
7. Add DirectX8\Inc\ to the top of the include files list.
8. Add DirectX8\Lib\ to the top of the library files list.
9. Save the workspace and build with Build -> Rebuild All.
10. Inside the System folder, run "del *.u & ucc make -nobind"

COMPILING THE SOURCE CODE UNDER WINDOWS 7/10/11
It needs to be added yet

# Features #

It needs to be added yet

# Authors #

- Ozymandias81: Archive Maintainer, Programmer
- neoxaero: Discord Coordinator Lead
More details needs to be included

# Known Bugs #

DO NOT PRESS F10 IN GAME, IT TRIES TO SAVE A SCREENSHOT BUT INSTEAD IT FREEZES THE APP
Obviously the game is not complete at all, it is very unstable and you can't save properly the game in several situations

# License #

It needs to be added yet

# EOF #

Join our Discord Channel for a better feedback: https://discord.gg/ZxaexEwgSv

==> ALWAYS BET ON DUKE COMMUNITY <==