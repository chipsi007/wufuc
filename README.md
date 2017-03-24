### Here's a list of the Windows updates that I will be talking about in this paper:

Title | Products | Classification | Last Updated | Version | Size
----- | -------- | -------------- | ------------ | ------- | ----
March, 2017 Preview of Monthly Quality Rollup for Windows 7 ([KB4012218]) | Windows 7 | Updates | 3/16/2017 | n/a | 93.4 MB
March, 2017 Preview of Monthly Quality Rollup for Windows 7 for x64-based Systems ([KB4012218]) | Windows 7 | Updates | 3/16/2017 | n/a |	153.9 MB
March, 2017 Preview of Monthly Quality Rollup for Windows Server 2008 R2 x64 Edition ([KB4012218]) | Windows Server 2008 R2 | Updates | 3/16/2017 | n/a | 153.9 MB
March, 2017 Preview of Monthly Quality Rollup for Windows 8.1 ([KB4012219]) |	Windows 8.1 | Updates | 3/16/2017 | n/a | 121.2 MB
March, 2017 Preview of Monthly Quality Rollup for Windows 8.1 for x64-based Systems ([KB4012219]) |	Windows 8.1 | Updates | 3/16/2017 | n/a | 218.0 MB
March, 2017 Preview of Monthly Quality Rollup for Windows Server 2012 R2 ([KB4012219]) | Windows Server 2012 R2 | Updates | 3/16/2017 | n/a | 218.0 MB

## About

After reading [this article](https://www.ghacks.net/2017/03/22/kb4012218-kb4012219-windows-update-processor-generation-detection/) on gHacks, I was inspired to look into these new rollup updates that Microsoft released on March 16. Among other things included in these updates, the changelog mentions the following:

> Enabled detection of processor generation and hardware support when PC tries to scan or download updates through Windows Update.

Which is just Microsoft's nice way of telling everyone who'd rather keep using Windows 7 or 8.1 to fuck themselves. _There have even been people with AMD and Intel systems from 2015 who have reportedly been locked out of Windows Update because of this!_

## Bad Microsoft!

Well then, time figure out how to fix this garbage.

I started by downloading the `.msu` package for my system (in my case, it was `windows6.1-kb4012218-x64_590943c04550a47c1ed02d3a040d325456f03663.msu`)

I extracted it using the command line `expand` tool, like this:

```bat
md "windows6.1-kb4012218-x64"
expand -f:* ".\windows6.1-kb4012218-x64_590943c04550a47c1ed02d3a040d325456f03663.msu" ".\windows6.1-kb4012218-x64"
cd ".\windows6.1-kb4012218-x64"
md "Windows6.1-KB4012218-x64"
expand -f:* ".\Windows6.1-KB4012218-x64.cab" ".\Windows6.1-KB4012218-x64"
```

Great, now there's thousands of `.exe` and `.dll` files to sort through! Just kidding. Sort of. Maybe. :thinking:

I ended up using PowerShell to sort through all the binaries, like so:

```powershell
Get-ChildItem -Filter "wu*" -Exclude "*.mui" -Recurse | ForEach-Object { $_.FullName }
```

That's narrowed it down quite a bit! This is now what we're looking at:

- `wu.upgrade.ps.dll`
- `wuapi.dll`
- `wudriver.dll`
- `wups.dll`
- `wuapp.exe`
- `wuwebv.dll`
- `wuauclt.exe`
- `wuaueng.dll`
- `wups2.dll`
- `wucltux.dll`
- `wuapi.dll`
- `wudriver.dll`
- `wups.dll`
- `wuapp.exe`
- `wuwebv.dll`

Next, I started comparing these binaries with the ones already on my system with [BinDiff] and [Diaphora], starting with `wuauclt.exe`. After turning up empty with that (the two binaries were nearly identical), I decided to take a look at `wuaueng.dll`, which turned up quite a few relevant new functions:

EA | Name | Basicblock | Instructions | Edges 
-- | ---- | ---------- | ------------ | -----
`00000600001DCB9C` | ``CWUTelemetryDownloadCanceledEvent::FireAsimovEvent(void)`` | 36 | 446 | 53
`00000600001D8F98` | ``CWUTelemetryDownloadCanceledEvent::`scalar deleting destructor'(uint)`` | 3 | 15 | 3
`00000600001D8FD0` | ``CWUTelemetryDownloadEvent::CWUTelemetryDownloadEvent(void)`` | 1 | 58 | 0
`00000600001DAEDC` | ``CWUTelemetryDownloadEvent::Init(CReporter *,long,long,ushort const *,long,_GUID,_GUID,CReportingOptionalValues &,AsimovDataInAddition *)`` | 6 | 50 | 8
`00000600001DAFB8` | ``CWUTelemetryDownloadEvent::InitializeMemebersFromOptionalData(tagOptionalData *)`` | 27 | 91 | 40
`00000600001D9100` | ``CWUTelemetryDownloadEvent::~CWUTelemetryDownloadEvent(void)`` | 2 | 60 | 1
`00000600001DC2C4` | ``CWUTelemetryDownloadFailedEvent::FireAsimovEvent(void)`` | 36 | 446 | 53
`00000600001DB114` | ``CWUTelemetryDownloadStartedEvent::FireAsimovEvent(void)`` | 36 | 446 | 53
`00000600001DB9EC` | ``CWUTelemetryDownloadSucceededEvent::FireAsimovEvent(void)`` | 36 | 446 | 53
`00000600001D8C48` | ``CWUTelemetryEventFactory::FireTelemetryEvent(CReporter *,long,long,ushort const *,long,_GUID,_GUID,CReportingOptionalValues &,AsimovDataInAddition *)`` | 11 | 76 | 17
`00000600001D8574` | ``CWUTelemetryEventFactory::GetTelemetryEvent(CReporter *,long,long,ushort const *,long,_GUID,_GUID,CReportingOptionalValues &,AsimovDataInAddition *,CWUTelemetryEvent * *)`` | 77 | 395 | 127
`00000600001DEE7C` | ``CWUTelemetryInstallCanceledEvent::FireAsimovEvent(void)`` | 34 | 409 | 50
`00000600001D8DD4` | ``CWUTelemetryInstallEvent::CWUTelemetryInstallEvent(void)`` | 1 | 57 | 0
`00000600001DD474` | ``CWUTelemetryInstallEvent::Init(CReporter *,long,long,ushort const *,long,_GUID,_GUID,CReportingOptionalValues &,AsimovDataInAddition *)`` | 6 | 50 | 8
`00000600001DD550` | ``CWUTelemetryInstallEvent::InitializeMemebersFromOptionalData(tagOptionalData *)`` | 23 | 81 | 34
`00000600001D8EFC` | ``CWUTelemetryInstallEvent::~CWUTelemetryInstallEvent(void)`` | 2 | 66 | 1
`00000600001DE67C` | ``CWUTelemetryInstallFailedEvent::FireAsimovEvent(void)`` | 34 | 409 | 50
`00000600001DF67C` | ``CWUTelemetryInstallRebootPendingEvent::FireAsimovEvent(void)`` | 34 | 409 | 50
`00000600001D8D9C` | ``CWUTelemetryInstallRebootPendingEvent::`scalar deleting destructor'(uint)`` | 3 | 15 | 3
`00000600001DD67C` | ``CWUTelemetryInstallStartedEvent::FireAsimovEvent(void)`` | 34 | 409 | 50
`00000600001DDE7C` | ``CWUTelemetryInstallSucceededEvent::FireAsimovEvent(void)`` | 34 | 409 | 50
`00000600001CAE68` | ``CWUTelemetryScanFailedEvent::FireAsimovEvent(void)`` | 31 | 416 | 46
`00000600001CA100` | ``CWUTelemetryScanRetryEvent::FireAsimovEvent(void)`` | 9 | 108 | 13
`00000600001CA588` | ``CWUTelemetryScanSucceededEvent::FireAsimovEvent(void)`` | 47 | 459 | 73
`00000600001CB790` | ``CWUTelemetryUnsupportedSystemClickSupportEvent::FireAsimovEvent(void)`` | 5 | 22 | 7
`00000600001CB9B0` | ``CWUTelemetryUnsupportedSystemClickSupportEvent::`scalar deleting destructor'(uint)`` | 3 | 17 | 3
`00000600001CB7FC` | ``CWUTelemetryUnsupportedSystemDetectionEvent::FireAsimovEvent(void)`` | 5 | 22 | 7
`00000600001CB970` | ``CWUTelemetryUnsupportedSystemDetectionEvent::`scalar deleting destructor'(uint)`` | 3 | 17 | 3
`00000600001CB724` | ``CWUTelemetryUnsupportedSystemNotificationDismissEvent::FireAsimovEvent(void)`` | 5 | 22 | 7
`00000600001CB9F0` | ``CWUTelemetryUnsupportedSystemNotificationDismissEvent::`scalar deleting destructor'(uint)`` | 3 | 17 | 3
`00000600001CB6B8` | ``CWUTelemetryUnsupportedSystemNotificationShowEvent::FireAsimovEvent(void)`` | 5 | 22 | 7
`00000600001CBA30` | ``CWUTelemetryUnsupportedSystemNotificationShowEvent::`scalar deleting destructor'(uint)`` | 3 | 17 | 3
**`0000060000102F08`** | **``IsCPUSupported(void)``** | **20** | **157** | **31**
**`00000600000AF3C0`** | **``IsDeviceServiceable(void)``** | **7** | **31** | **8**
`00000600000832CC` | ``TraceLoggingEnableForTelemetry(_TlgProvider_t const *)`` | 16 | 86 | 23
`0000060000083210` | ``TraceLoggingSetInformation(_TlgProvider_t const *,_EVENT_INFO_CLASS,void *,ulong)`` | 6 | 50 | 8

We have found culprits,  [`IsDeviceServiceable(void)`](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185) and [`IsCPUSupported(void)`](https://gist.github.com/zeffy/1a8f8984d2bec97ae24af63a76278694)!

## Solutions

`IsCPUSupported(void)` is only ever called by `IsDeviceServiceable(void)`, which is called by five other functions. Luckily, there are a few ways to kill this CPU check.

1. Patch `wuaueng.dll` and change `dword_600002EE948` (see [this line](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185#file-isdeviceserviceable-c-L7)) which is at file offset `0x26C948`, from `0x01` to `0x00`, which makes `IsDeviceServiceable(void)` jump over its entire body and return 1 (supported CPU) immediately. This is my preferred method, because it is the least intrusive and doesn't require any runtime memory patching. **These offsets are only for the Windows 7 x64 version, I will upload `.xdelta` files for all of the other versions eventually.** The only downside of this method is you have to re-apply the patch whenever `wuaueng.dll` gets updated. 
2. `nop` out all the instructions highlighted [here](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185#file-isdeviceserviceable-asm-L24-L26) in `IsDeviceServiceable(void)`, this will enable the usage of the `ForceUnsupportedCPU` of type `REG_DWORD` under the registry key `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\WindowsUpdate\Test\Scan` (if you want to use this method, you will probably have to create it). Set this value to `0x00000001` to force unsupported CPUs, and back to `0x00000000` to change the behaviour back to default. **This behaviour is undocumented and could be removed in future updates.**
3. I guess you could do runtime memory patching, but that's ugly.

[KB4012218]: https://www.catalog.update.microsoft.com/search.aspx?q=kb4012218
[KB4012219]: https://www.catalog.update.microsoft.com/search.aspx?q=kb4012219
[BinDiff]: https://www.zynamics.com/software.html
[Diaphora]: http://diaphora.re
