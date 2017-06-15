# wufuc [![](https://img.shields.io/badge/formerly-kb4012218--19-blue.svg)](../../tree/old-kb4012218-19 "formerly kb4012218-19") [![](https://ci.appveyor.com/api/projects/status/0s2unkpokttyslf0?svg=true)](https://ci.appveyor.com/project/zeffy/wufuc "AppVeyor build status")

<a href='https://pledgie.com/campaigns/34055'><img alt='Click here to lend your support to: wufuc - Help support development and make a donation at pledgie.com !' src='https://pledgie.com/campaigns/34055.png?skin_name=chrome' border='0' ></a>

Disables the "Unsupported Hardware" message in Windows Update, and allows you to continue installing updates on Windows 7 and 8.1 systems with Intel Kaby Lake, AMD Ryzen, or other unsupported processors.

## Downloads [![](https://img.shields.io/github/downloads/zeffy/wufuc/total.svg)](../../releases "Total downloads")

### You can get the latest stable version [here](../../releases/latest)!

If you are feeling brave, you can try the latest unstable builds [here](https://ci.appveyor.com/project/zeffy/wufuc). **Use these at your own risk!**

## Reporting an issue [![](http://isitmaintained.com/badge/resolution/zeffy/wufuc.svg)](http://isitmaintained.com/project/zeffy/wufuc "Average time to resolve an issue")

There isn't really a way for wufuc to directly interact with your desktop, because it runs outside the context of a normal user session. Therefore you must first download and run another program called [DebugView](https://technet.microsoft.com/en-us/sysinternals/debugview.aspx) to display wufuc's debug messages. These logs are absolutely crucial if you want me to be able to resolve your issue as quickly as possible, so please include them if you can.

#### The best way to get a log of the entire life-cycle of wufuc is to do the following:

1. Disable wufuc by running `disable_wufuc.bat` as administrator.
2. Start `Dbgview.exe` as administrator and check `Capture -> Capture Global Win32`.
3. Enable wufuc by running `enable_wufuc.bat` as administrator.
4. Start the `wuauserv` service if it isn't already running.
5. Output will be shown in DebugView, copy/paste this text into your issue.

#### Other helpful information to include when reporting issues:

- What happened? What did you expect to happen instead?
- What version of wufuc are you using?
- What version of Windows are you using? x86 or x64?
- Were there any errors reporting during installation? What were they?
- What is the file version or SHA-1 hash of `C:\Windows\System32\wuaueng.dll`?
- Any other information you feel is relevant to your issue.

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

- At system boot the wufuc scheduled task runs as the `NT AUTHORITY\SYSTEM` user.
- `wufuc` determines what service host group process the Windows Update service runs in (typically `netsvcs`), and injects itself into it.
- Once injected, it applies a hook to `LoadLibraryEx` that automatically patches `wuaueng.dll` when it is loaded.
- Any previously loaded `wuaueng.dll` is also patched.

### Several improvements over my batchfile method:

- **No system files are modified!**
- Heuristic-based patching, which means it will usually keep working even after updates.		
- C is best language.		
- No external dependencies.

## Q & A

### How to install/uninstall?

Just download the [latest release](../../releases/latest), and extract the `wufuc` folder to a permanent location (like `C:\Program Files\wufuc`) and then run `install_wufuc.bat` as administrator. 

To uninstall run `uninstall_wufuc.bat` as administrator.

### How to update when a new version comes out?

Unless otherwise noted, you should only have to:

- Run `disable_wufuc.bat` as administrator.
- Copy the new files into the install folder, overwriting the old ones.
- Run the new `install_wufuc.bat` as administrator.

If you run into problems, try doing a full uninstall/reinstall.

### How do I remove your old patch and use this instead?

I've included a utility script called `repair_wuaueng.dll.bat`. When you run it, it will initiate an `sfc` scan and revert any changes made to `wuaueng.dll`.
