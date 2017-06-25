# Contributing guidelines

## Reporting an issue [![](https://isitmaintained.com/badge/resolution/zeffy/wufuc.svg)](https://isitmaintained.com/project/zeffy/wufuc)

#### Before you create an issue, please make sure of the following:

- Are you using at least the [latest stable version](../../releases/latest)?
- If you previously used any of the old batchfile patches, did you revert it by running `repair_wuaueng.dll.bat`?
- Have you tried restarting your computer?

#### After you've confirmed those things, get a complete debug log by doing the following:

1. Download a program from Microsoft called [DebugView](https://technet.microsoft.com/en-us/sysinternals/debugview.aspx).
2. Start `Dbgview.exe` as administrator and check `Capture -> Capture Global Win32`.
3. Restart wufuc and `wuauserv` by running `debugview_helper.bat` as administrator.
4. Output will be shown in DebugView, copy/paste this text into your issue.

This is necessary because there isn't really a way for wufuc to directly interact with your desktop to show you error messages, as it runs outside the context of a normal user session, just like a system service.

Any new issues that don't have debug logs (when relevant) will be closed immediately and the poster directed to the contributing guidelines.

#### Other helpful information to include when reporting issues:

- What build are you using? Stable release or unstable AppVeyor builds?
- What version of Windows are you using? (e.g., Windows 7 x64)
- What is the file version or SHA-1 hash of `C:\Windows\System32\wuaueng.dll`?
- Any other information you feel is relevant to your issue.
