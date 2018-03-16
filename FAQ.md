# Frequently Asked Questions

## How to deploy wufuc using Group Policy

[There is a tutorial on the Advanced Installer website that explains how to do this](http://www.advancedinstaller.com/user-guide/tutorial-gpo.html).

## How to use unattended feature in the batch setup scripts

`install_wufuc.bat` and `uninstall_wufuc.bat` both support two command line parameters that can be used alone, or combined to change the behavior of the scripts:

- `/NORESTART` - Automatically declines rebooting after the setup finishes.
- `/UNATTENDED` - Skips all prompts for user interaction, and automatically restarts unless `/NORESTART` is also specified.

These must be used from an elevated command line prompt.

## How to restore Windows Update to its default configuration

Windows Update by default is configured to run as `SERVICE_WIN32_SHARE_PROCESS`, which means it shares a single `svchost.exe` process with several other Windows services.
It can also be configured as `SERVICE_WIN32_OWN_PROCESS`, which means it runs in its own process, which doesn't start until Windows Update does.

Normally, either of these configurations is supported.

However, if you have used another third-party solution for the CPU fix (such as UpdatePack7R2 or WuaCpuFix) either knowingly or unknowingly, wufuc will not function correctly if Windows Update is configured as `SERVICE_WIN32_OWN_PROCESS`.

You have two ways to work around this limitation, either:

- You can remove the other third-party solution yourself.
- Alternatively, go to the directory that you installed wufuc to, open the `Troubleshooting` subdirectory, and merge the `Restore_wuauserv.reg` file with your registry by double-clicking it, and then restart your PC.

## How to manually remove wufuc v0.8.0.143 when it is impossible to uninstall it normally

This applies exclusively to a very buggy version of wufuc that was only available for download for a short period of time, other versions are unaffected.

1. [Boot into Safe Mode with Command Prompt](https://support.microsoft.com/en-us/help/17419/windows-7-advanced-startup-options-safe-mode).
2. In the command prompt type `regedit` and press enter.
3. Navigate to the key `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options`
4. Expand the `Image File Execution Options` tree.
5. Locate the `svchost.exe` sub key, right-click it and press **Delete**.
6. Reboot, and you should be able to log in normally again.
7. Open Add and Remove Programs, locate and run the normal wufuc uninstaller to complete the removal process.
