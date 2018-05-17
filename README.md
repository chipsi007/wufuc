# wufuc
[![Donate Bitcoin](https://cdn.rawgit.com/zeffy/wufuc/badges/bitcoin.svg)](https://admin.gear.mycelium.com/gateways/3554/orders/new)
[![AppVeyor Builds](https://img.shields.io/appveyor/ci/zeffy/wufuc.svg?logo=appveyor&style=flat-square)][AppVeyor]
[![All Releases](https://img.shields.io/github/downloads/zeffy/wufuc/total.svg?style=flat-square)][Latest]
[![Chat on Discord](https://img.shields.io/discord/421524706043101194.svg?label=discord&logo=discord&colorA=7078C2&colorB=7B81D8&style=flat-square)](https://discord.gg/G8PD2Wa)

[:de:] [:es:] [:fr:] [:it:] [:hungary:] [:brazil:] [:ru:] [:cn:] [:taiwan:]

Disables the "Unsupported Hardware" message in Windows Update, and allows you to continue installing updates on Windows 7 and 8.1 systems with Intel Kaby Lake, AMD Ryzen, or other unsupported processors.

## Downloads 

**[Latest stable build][Latest] - Most people will want this version.**

[Unstable builds][AppVeyor] - Probably contains bugs; do not report issues with these builds.

## Donate :heart:

[**Click here for donation options!**](https://github.com/zeffy/wufuc/blob/master/DONATE.md)

## Background

The release notes for Windows updates KB4012218 and KB4012219 included the following:

> Enabled detection of processor generation and hardware support when PC tries to scan or download updates through Windows Update.

These updates marked the implementation of a [policy change](https://blogs.windows.com/windowsexperience/2016/01/15/windows-10-embracing-silicon-innovation/) they announced some time ago, where Microsoft stated that they would not be supporting Windows 7 or 8.1 on next-gen Intel, AMD and Qualcomm processors.

This is essentially a big middle finger to anyone who decides to not "upgrade" to Windows 10,
and it is especially unfortunate considering the extended support periods for Windows 7 and 8.1 won't be ending until January 4, 2020 and January 10, 2023 respectively.

Some people with older Intel and AMD processors are also affected! I've received user reports of the following CPUs all being blocked from receiving updates:

- [Intel Atom Z530](https://github.com/zeffy/wufuc/issues/7)
- [Intel Atom D525](https://github.com/zeffy/wufuc/issues/34)
- [Intel Core i5-M 560](https://github.com/zeffy/wufuc/issues/23)
- [Intel Core i5-4300M](https://github.com/zeffy/wufuc/issues/24)
- [Intel Core i7-4930K](https://github.com/zeffy/wufuc/issues/126)
- [Intel Pentium B940](https://github.com/zeffy/wufuc/issues/63)
- [AMD FX-6300](https://github.com/zeffy/wufuc/issues/135#issuecomment-367054217)
- [AMD FX-8350](https://github.com/zeffy/wufuc/issues/32)
- [AMD Turion 64 Mobile Technology ML-34](https://github.com/zeffy/wufuc/issues/80)

## Features

- Enables Windows Update on PCs with unsupported processors.
- Written in C, the best programming language. :sunglasses:
- Completely free (as in freedom) software.
- Does not modify any system files.
- Byte pattern-based patching, which means it will usually keep working even after new updates come out.
- No dependencies.

## How wufuc works

The tl;dr version is basically:

* Inside a system file called `wuaueng.dll`, there are two functions responsible for the CPU check: `IsDeviceServiceable` and `IsCPUSupported`.
* `IsDeviceServiceable` simply calls `IsCPUSupported` once, and then saves the result and re-uses it on subsequent calls.
* I take advantage of this behavior in wufuc by patching the saved result so that it is always `TRUE`, or supported.

If you would like more information, you can read my original write-up on discovering the CPU check [here](https://github.com/zeffy/wufuc/tree/old-kb4012218-19).


## Building

To build wufuc from source, you need to download and install the following:

1. [Visual Studio 2017](https://www.visualstudio.com/).
2. [Windows Driver Kit (WDK)](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk).
3. (Optional, for MSI packages) [Advanced Installer](https://www.advancedinstaller.com/).

## Sponsors

### [Advanced Installer](https://www.advancedinstaller.com/)

The installer packages are created with Advanced Installer using an [open source license](https://www.advancedinstaller.com/free-license.html). 
Advanced Installer's intuitive and friendly user interface allowed me to quickly create a feature complete installer with minimal effort. Check it out!

## Special thanks

- [**@wj32**](https://github.com/wj32) for his awesome program [Process Hacker](https://github.com/processhacker2/processhacker), and also for his [phnt headers](https://github.com/processhacker2/processhacker/tree/master/phnt).
- [**@mrexodia**](https://github.com/mrexodia) for [x64dbg](https://github.com/x64dbg/x64dbg), its [`patternfind.cpp`](https://github.com/x64dbg/x64dbg/blob/development/src/dbg/patternfind.cpp) algorithm, and its issue template which I adapted for this project.

[Latest]: https://github.com/zeffy/wufuc/releases/latest
[AppVeyor]: https://ci.appveyor.com/project/zeffy/wufuc
[:de:]: https://github.com/zeffy/wufuc/wiki/README-(Deutsch)
[:es:]: https://github.com/zeffy/wufuc/wiki/README-(Espa%C3%B1ol)
[:fr:]: https://github.com/zeffy/wufuc/wiki/README-(Fran%C3%A7ais)
[:it:]: https://github.com/zeffy/wufuc/wiki/README-(Italiano)
[:hungary:]: https://github.com/zeffy/wufuc/wiki/README-(Magyar)
[:brazil:]: https://github.com/zeffy/wufuc/wiki/README-(Portugu%C3%AAs-Brasileiro)
[:ru:]: https://github.com/zeffy/wufuc/wiki/README-(%D1%80%D1%83%D1%81%D1%81%D0%BA%D0%B8%D0%B9)
[:cn:]: https://github.com/zeffy/wufuc/wiki/README-(%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87)
[:taiwan:]: https://github.com/zeffy/wufuc/wiki/README-(%E7%B9%81%E9%AB%94%E4%B8%AD%E6%96%87)
