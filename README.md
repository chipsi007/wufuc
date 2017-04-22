### [Click here if you are looking for the latest patch files!](https://github.com/zeffy/kb4012218-19/releases)

---

### [Click here for a list of Windows updates supported by this patch](docs/Supported_Updates.md)

---

## Preface

After reading [this article on gHacks](https://www.ghacks.net/2017/03/22/kb4012218-kb4012219-windows-update-processor-generation-detection/), I was inspired to look into these new rollup updates that Microsoft released on March 16. Among other things, the changelog mentions the following:

> Enabled detection of processor generation and hardware support when PC tries to scan or download updates through Windows Update.

This update marked the implementation of a [policy change](https://blogs.windows.com/windowsexperience/2016/01/15/windows-10-embracing-silicon-innovation/) they announced some time ago, where Microsoft stated that they would not be supporting Windows 7 or 8.1 on next-gen Intel, AMD and Qualcomm processors. 

This is essentially a giant middle finger to anyone who dare not "upgrade" to the steaming pile of garbage known as Windows 10. Especially considering the extended support periods for Windows 7 and 8.1 still have a few years left, and will be coming to a close in 2020 and 2024 respectively.

There have even been people with older Intel and AMD systems who have been locked out of Windows Update because of these updates (see [#7](/../../issues/7) and [this](https://answers.microsoft.com/en-us/windows/forum/windows8_1-update/amd-carrizo-ddr4-unsupported-hardware-message-on/f3fb2326-f413-41c9-a24b-7c14e6d51b0c?tab=question&status=AllReplies)).

## Bad Microsoft!

I started by downloading the update package for my system (in my case, it was `windows6.1-kb4012218-x64_590943c04550a47c1ed02d3a040d325456f03663.msu`)

I extracted it using the command line `expand` tool:

```bat
md "windows6.1-kb4012218-x64"
expand -f:* ".\windows6.1-kb4012218-x64_590943c04550a47c1ed02d3a040d325456f03663.msu" ".\windows6.1-kb4012218-x64"
cd ".\windows6.1-kb4012218-x64"
md "Windows6.1-KB4012218-x64"
expand -f:* ".\Windows6.1-KB4012218-x64.cab" ".\Windows6.1-KB4012218-x64"
```

Great, now there's thousands of files to sort through! Just kidding. Sort of. Maybe. :thinking:

I ended up using PowerShell to sort through and filter out all the binaries that weren't related to Windows Update, like so:

```powershell
Get-ChildItem -Filter "wu*" -Exclude "*.mui" -Recurse | ForEach-Object { $_.FullName }
```

That narrowed it down to 14 files, excellent!

Next, I started comparing these binaries with the ones already on my system with [BinDiff] and [Diaphora]. I eventually got to `wuaueng.dll`, which turned up quite a few interesting new functions:

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

We have found culprits, [`IsDeviceServiceable(void)`](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185) and [`IsCPUSupported(void)`](https://gist.github.com/zeffy/1a8f8984d2bec97ae24af63a76278694)!

## Solutions

Luckily, there are a couple of different ways to kill this CPU check by patching `wuaueng.dll`.

1. Change the value of `dword_600002EE948` (see [this line](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185#file-isdeviceserviceable-c-L7)) which is at file offset `0x26C948`, from `0x01` to `0x00`. This makes `IsDeviceServiceable(void)` skip over the entire CPU check and immediately return the value stored at `dword_600002EE94C`, which by default is 1 (supported CPU). This is my preferred method, as it is a simple 1-byte change. **Note: this offset is only for the KB4012218-x64, for a list of all the patch offsets [click here](docs/Patch_Offsets.md).**

2. Fill all the instructions highlighted [here](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185#file-isdeviceserviceable-asm-L24-L26) in `IsDeviceServiceable(void)` with `nop`s. This will enabling using the `ForceUnsupportedCPU` value of type `REG_DWORD` under the registry key `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\WindowsUpdate\Test\Scan` (you will most likely have to create this registry key). You can set this value to `0x00000001` to force unsupported CPUs, or to `0x00000000` to use the default behaviour. You will probably need to restart your PC or restart the `wuauserv` service in order for changes to apply. **This is an internal testing feature used by Microsoft and could be removed in future updates, so I will not be providing xdelta files for it.**

## Caveats

- You have to apply a new patch whenever `wuaueng.dll` gets updated.
- SFC scan errors will most likely occur as it will believe the integrity of the system has been compromised.

[BinDiff]: https://www.zynamics.com/software.html
[Diaphora]: http://diaphora.re
