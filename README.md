# wufuc [![a.k.a. kb4012218-19](https://img.shields.io/badge/a.k.a.-kb4012218--19-blue.svg)](../../tree/old-kb4012218-19) [![download count](https://img.shields.io/github/downloads/zeffy/kb4012218-19/total.svg)](https://github.com/zeffy/kb4012218-19/releases/latest)

Disables the "Unsupported Hardware" message in Windows Update, and allows you to continue installing updates on Windows 7 and 8.1 systems with Intel Kaby Lake, AMD Ryzen, or other unsupported processors.

## Preface

The changelog for Windows updates KB4012218 and KB4012219 included the following:

> Enabled detection of processor generation and hardware support when PC tries to scan or download updates through Windows Update.

These updates marked the implementation of a [policy change](https://blogs.windows.com/windowsexperience/2016/01/15/windows-10-embracing-silicon-innovation/) they announced some time ago, where Microsoft stated that they would not be supporting Windows 7 or 8.1 on next-gen Intel, AMD and Qualcomm processors. 

It was essentially a big middle finger to anyone who decides to not "upgrade" to the steaming pile of :poop: known as Windows 10. Especially considering the extended support periods for Windows 7 and 8.1 won't be ending until January 4, 2020 and January 10, 2023 respectively.

There have even been people with older Intel and AMD systems who have been locked out of Windows Update because of these updates (see [#7](../../issues/7) and [this](https://answers.microsoft.com/en-us/windows/forum/windows8_1-update/amd-carrizo-ddr4-unsupported-hardware-message-on/f3fb2326-f413-41c9-a24b-7c14e6d51b0c?tab=question&status=AllReplies)).

## Bad Microsoft!

If you are interested, you can read my original write up on discovering the CPU check [here](../../tree/old-kb4012218-19).

Basically, inside a file called `wuaueng.dll` there are two functions: [`IsDeviceServiceable(void)`](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185) and [`IsCPUSupported(void)`](https://gist.github.com/zeffy/1a8f8984d2bec97ae24af63a76278694). `IsDeviceServiceable(void)` is essentially a wrapper around `IsCPUSupported(void)` that caches the result it recieves and recycles it on subsequent calls. 

My patch takes advantage of this result caching behavior by setting the "hasn't run once" value to `FALSE` and the cached result to `TRUE`.

## How it works

- On system boot the `wufuc` scheduled task runs under the `NT AUTHORITY\SYSTEM` user.
- `wufuc` determines what service host process the Windows Update service (`wuauserv`) runs in, and injects itself into it.
- Once injected, it applies a hook to `LoadLibraryEx` that automatically patches `wuaueng.dll` when it is loaded.
- Any previously loaded `wuaueng.dll` is also patched.

### Several improvements over my script-based approach:		
- **No system files are modified!***
- Heuristic byte signature patching persists over new updates.		
- C is best language.		
- No external dependencies except for Microsoft Visual C++ 2015 Redistributable.	

### How to install/uninstall?

Just run move the `wufuc` folder to wherever you want and run `install_wufuc.bat` as administrator.

To uninstall run `uninstall_wufuc.bat` as administrator. 

To temporarily disable the patch, just go to the Task Scheduler and disable the `wufuc.{72EEE38B-9997-42BD-85D3-2DD96DA17307}` task, then restart your computer.

### How do I remove your old patch and use this instead?

I've included a utility script called `repair_wuaueng.dll.bat` that will initiate an sfc scan and attempt to automatically revert any changes made to `wuaueng.dll`.
