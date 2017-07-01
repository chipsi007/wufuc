# wufuc [![](https://ci.appveyor.com/api/projects/status/0s2unkpokttyslf0?svg=true)](https://ci.appveyor.com/project/zeffy/wufuc) [![Click here to tip wufuc on Gratipay!](https://img.shields.io/gratipay/team/wufuc.svg)](https://gratipay.com/wufuc/)

[![Click here to lend your support to wufuc and make a donation at pledgie.com !](https://pledgie.com/campaigns/34055.png)](https://pledgie.com/campaigns/34055)

Disables the "Unsupported Hardware" message in Windows Update, and allows you to continue installing updates on Windows 7 and 8.1 systems with Intel Kaby Lake, AMD Ryzen, or other unsupported processors.

## Downloads [![](https://img.shields.io/github/downloads/zeffy/wufuc/total.svg)](../../releases)

### You can get the latest stable version [here](../../releases/latest)!

If you are feeling brave, you can try the latest unstable builds [here](https://ci.appveyor.com/project/zeffy/wufuc). **Use these at your own risk!**

## Reporting an issue [![](https://isitmaintained.com/badge/resolution/zeffy/wufuc.svg)](https://isitmaintained.com/project/zeffy/wufuc)

See [CONTRIBUTING.md](CONTRIBUTING.md).

## Preface

The changelog for Windows updates KB4012218 and KB4012219 included the following:

> Enabled detection of processor generation and hardware support when PC tries to scan or download updates through Windows Update.

These updates marked the implementation of a [policy change](https://blogs.windows.com/windowsexperience/2016/01/15/windows-10-embracing-silicon-innovation/) they announced some time ago, where Microsoft stated that they would not be supporting Windows 7 or 8.1 on next-gen Intel, AMD and Qualcomm processors.

It was essentially a big middle finger to anyone who decides to not "upgrade" to the steaming pile of :shit: known as Windows 10, especially considering the extended support periods for Windows 7 and 8.1 won't be ending until January 4, 2020 and January 10, 2023 respectively.

This has even affected people with older Intel and AMD processors! I've received user reports of the [Intel Atom Z530](../../issues/7), [Intel Core i5-M 560](../../issues/23), [Intel Core i5-4300M](../../issues/24), [Intel Atom D525](../../issues/34), [Intel Pentium B940](../../issues/63), and [AMD FX-8350](../../issues/32) all being blocked from receiving updates.

## Bad Microsoft!

If you are interested, you can read my original write up on discovering the CPU check [here](../../tree/old-kb4012218-19).

## How it works

Basically, inside a file called `wuaueng.dll` there are two functions: [`IsDeviceServiceable(void)`](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185) and [`IsCPUSupported(void)`](https://gist.github.com/zeffy/1a8f8984d2bec97ae24af63a76278694). `IsDeviceServiceable(void)` is essentially a wrapper around `IsCPUSupported(void)` that caches the result it receives and recycles it on subsequent calls. 

My patch takes advantage of this result caching behavior by setting the "first run" value to `FALSE` and the cached result to `TRUE`.

- At system boot the wufuc scheduled task runs as the `NT AUTHORITY\SYSTEM` user.
- `wufuc` determines what service host group process the Windows Update service runs in (typically `netsvcs`), and injects itself into it.
- Once injected, it applies a hook to `LoadLibraryEx` that automatically patches `wuaueng.dll` when it is loaded.
- Any previously loaded `wuaueng.dll` is also patched.

### Several improvements over my batchfile method:

- **No system files are modified!**
- Heuristic-based patching, which means it will usually keep working even after new updates come out.		
- C is best language.		
- No external dependencies.

## Q & A

### How to install/uninstall?

Just download the [latest release](../../releases/latest), and extract the `wufuc` folder to a permanent location (like `C:\Program Files\wufuc`) and then run `install_wufuc.bat` as administrator. 

To uninstall run `uninstall_wufuc.bat` as administrator.

### How to update when a new version comes out?

Unless otherwise noted, you should only have to:

- Run `uninstall_wufuc.bat` as administrator.
- Copy the new files into the install folder, overwriting the old ones.
- Run the new `install_wufuc.bat` as administrator.

### How do I remove your old patch and use this instead?

I've included a utility script called `repair_wuaueng.dll.bat`. When you run it, it will initiate an `sfc` scan and revert any changes made to `wuaueng.dll`.
