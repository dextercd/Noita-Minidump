# Noita Minidumps

Crash dumps can help figure out why the game crashes. `Noita_dev.exe`
creates crash dumps in the `crashes/` folder, but for some reason this
is not enabled for `Noita.exe`.

This Noita mod enables crash dumps for any version of Noita. (GOG/
Humble/Steam, including older versions.)

Dumps are placed in `Noita/crashes/minidump_[YEAR-MONTH-DAY]T[TIME].dmp`.


## Analysis

WinDbg seems better for analysing crash dumps with no debugging
information. Unlike Visual Studio, it can reconstruct the call stack.

https://docs.microsoft.com/en-us/windows/win32/dxtecharts/crash-dump-analysis#debugging-a-minidump-with-windbg

In WinDbg can use CTRL+I to load the executable if Noita.exe is not in
the same path as the computer where the crashdump was created.
