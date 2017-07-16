# wufuc [![](https://ci.appveyor.com/api/projects/status/0s2unkpokttyslf0?svg=true)](https://ci.appveyor.com/project/zeffy/wufuc)

[English](README.md) | [русский](README.ru-RU.md) | [Français](README.fr-FR.md) | **Deutsch** | [Magyar](README.hu-HU.md) | [Portuguese (Brazil)](README.pt-BR.md)

[![Klicke hier um wufuc zu unterstützen und um eine Spende zu tätigen - auf pledgie.com !](https://pledgie.com/campaigns/34055.png)](https://pledgie.com/campaigns/34055) <a href='https://gratipay.com/wufuc/'><img height=37 alt='Click here to tip wufuc on Gratipay!' src='https://cdn.rawgit.com/zeffy/gratipay-badge/master/dist/gratipay.svg' /></a>

Das Tool schaltet die "Unsupported Hardware" Nachricht in Windows Update ab, und erlaub dir auf Windows 7 und 8.1 Systemen mit Intel Kaby Lake, AMD Ryzen, oder anderen nicht unterstützten Processoren weiter updates zu installieren.

## Downloads [![](https://img.shields.io/github/downloads/zeffy/wufuc/total.svg)](../../releases)

### Die neueste stabile version kannst du [hier](../../releases/latest) herunterladen!

Wenn du dich traust, kannst du von [hier](https://ci.appveyor.com/project/zeffy/wufuc) die letzten instabilen builds herunterladen. **Auf eingene Verantwortung!**

## Sponsoren

### [Advanced Installer](http://www.advancedinstaller.com/)
Die Installer-pakete wurden mit Advanced Installer unter einer open source licenz erstellt. Die intuitive und freundliche benutzeroberfläche von Advanced Installer hat mir erlaubt einenvollwertigen installer mit minimalem Aufwand zu erstellen. [Schaue es dir an!](http://www.advancedinstaller.com/)

## Fehler melden [![](https://isitmaintained.com/badge/resolution/zeffy/wufuc.svg)](https://isitmaintained.com/project/zeffy/wufuc)

Siehe [CONTRIBUTING.de-DE.md](CONTRIBUTING.de-DE.md).

## Vorwort

Der changelog für die Windows updates KB4012218 und KB4012219 enthielt das folgende:

> Enabled detection of processor generation and hardware support when PC tries to scan or download updates through Windows Update.

Damit wurde ein [verfahrenswechel](https://blogs.windows.com/windowsexperience/2016/01/15/windows-10-embracing-silicon-innovation/) eingeleitet, den sie vor einiger Zeit verkündeten. Microsoft hatte verkündet, dass sie Windows 7 oder 8.1 auf next-gen Intel, AMD und Qualcomm processoren nicht mehr unterstützen.

Wer also nicht auf Window 10 "upgraded" ist gea*****, obwohl der extended support für Windows 7 und 8.1 bis zum 4. Januar 2020, respektive zum 10. Januar 2023 läuft.

Dies betrifft sogar leute mit älteren Intel und AMD processoren!

## Böser Microsoft!

Wenn du interessiert bist, kannst du meinen Artikel lesen wie ich den CPU check gefunden habe -> [hier klicken](../../tree/old-kb4012218-19).

## Wie funktioniert es?

Im library file `wuaueng.dll` gibt es zwei funktionen: [`IsDeviceServiceable(void)`](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185) und [`IsCPUSupported(void)`](https://gist.github.com/zeffy/1a8f8984d2bec97ae24af63a76278694). `IsDeviceServiceable(void)` ist ein wrapper um `IsCPUSupported(void)` welche die empfangenen ergebnisse zwischenspeichert und bei neuen aufrufen wiederverwendet.

Mein patch nutzt dieses Verhalten aus und setzt die "first run" variable auf `FALSE` und den cached result auf `TRUE`.

- Beim booten wird vom `NT AUTHORITY\SYSTEM` Benutzer ein wufuc scheduled task gestartet.
- `wufuc` erkundet in welchem service host group Prozess der Windows Update service läuft (typischerweise `netsvcs`), und injiziert sich in den Prozess.
- Jetzt wird ein hook in `LoadLibraryEx` gesetzt, welche `wuaueng.dll` automatisch patcht wenn es geladen wird.
- Vorher geladene `wuaueng.dll` instanzen werden auch gepatcht.

### Diverse verbesserungen seit meiner batchfile Methode:

- **Es werden keine Systemfiles verändert!**
- Heuristisches patchen, wodurch der patch auch nach updates funktionieren sollte.
- Es wird C benutzt!
- Keine externen Abhängigkeiten.
