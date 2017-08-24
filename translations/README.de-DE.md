# wufuc [![](https://ci.appveyor.com/api/projects/status/0s2unkpokttyslf0?svg=true)](https://ci.appveyor.com/project/zeffy/wufuc)

[English](../README.md) | [русский](README.ru-RU.md) | [Français](README.fr-FR.md) | **Deutsch** | [Magyar](README.hu-HU.md) | [Portuguese (Brazil)](README.pt-BR.md)

[![Klicke hier um wufuc zu unterstützen und um eine Spende zu tätigen - auf pledgie.com !](https://pledgie.com/campaigns/34055.png)](https://pledgie.com/campaigns/34055) <a href='https://gratipay.com/wufuc/'><img height=37 alt='Click here to tip wufuc on Gratipay!' src='https://cdn.rawgit.com/zeffy/gratipay-badge/master/dist/gratipay.svg' /></a>

Das Tool schaltet die "Unsupported Hardware" Nachricht in Windows Update ab und erlaubt dir auf Windows 7 und 8.1 Systemen mit Intel Kaby Lake, AMD Ryzen oder anderen nicht unterstützten Prozessoren weiterhin Updates zu installieren.

## Downloads [![](https://img.shields.io/github/downloads/zeffy/wufuc/total.svg)](../../../releases)

### Die neuste stabile Version kannst du [hier](../../../releases/latest) herunterladen!

Wenn du dich traust, kannst du von [hier](https://ci.appveyor.com/project/zeffy/wufuc) die letzten instabilen Builds herunterladen. **Auf eigene Verantwortung!**

## Sponsoren

### [Advanced Installer](http://www.advancedinstaller.com/)
Die Installer-Pakete wurden mit Advanced Installer unter einer Open Source Lizenz erstellt. Die Intuitive und Freundliche Benutzeroberfläche von Advanced Installer hat es mir erlaubt einen vollwertigen Installer mit minimalem Aufwand zu erstellen. [Schaue es dir an!](http://www.advancedinstaller.com/)

## Fehler melden [![](https://isitmaintained.com/badge/resolution/zeffy/wufuc.svg)](https://isitmaintained.com/project/zeffy/wufuc)

Siehe [CONTRIBUTING.de-DE.md](CONTRIBUTING.de-DE.md).

## Vorwort

Der Changelog für die Windows Updates KB4012218 und KB4012219 enthielt folgendes:

> Enabled detection of processor generation and hardware support when PC tries to scan or download updates through Windows Update.

Damit wurde ein [Verfahrenswechel](https://blogs.windows.com/windowsexperience/2016/01/15/windows-10-embracing-silicon-innovation/) eingeleitet, den sie vor einiger Zeit verkündet hatten. Microsoft hatte verkündet, dass sie Windows 7 oder 8.1 auf Nächste Generation Intel, AMD und Qualcomm Prozessoren nicht mehr unterstützen werden.

Wer also nicht auf Windows 10 "upgraded" ist gea*****, obwohl der Erweiterte Support für Windows 7 und 8.1 bis zum 4. Januar 2020, respektiv zum 10. Januar 2023 läuft.

Das betrifft sogar Leute mit älteren Intel und AMD Prozessoren!

## Böses Microsoft!

Wenn du interessiert bist, kannst du meinen Artikel lesen wie ich den CPU Check herausgefunden habe -> [hier klicken](../../../tree/old-kb4012218-19).

## Wie funktioniert es?

In der Bibliothekdatei `wuaueng.dll` gibt es zwei Funktionen: [`IsDeviceServiceable(void)`](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185) und [`IsCPUSupported(void)`](https://gist.github.com/zeffy/1a8f8984d2bec97ae24af63a76278694). `IsDeviceServiceable(void)` ist ein Wrapper um `IsCPUSupported(void)` welche die Empfangenen Ergebnisse zwischenspeichert und bei neuen Aufrufen wiederverwendet.

Mein Patch nutzt dieses Verhalten aus und setzt die "Erstausführung" Variable auf `FALSCH` und den gecachten Ergebnis auf `WAHR`.

- Beim Booten wird vom `NT AUTHORITY\SYSTEM` Benutzer ein wufuc geplanter Task gestartet.
- `wufuc` erkundet im welchen Service Prozesshostgruppe der Windows Update Service läuft (typischerweise `netsvcs`) und injiziert sich in dem Prozess.
- Jetzt wird eine Hook in `LoadLibraryEx` gesetzt, welche `wuaueng.dll` automatisch patcht wenn es geladen wird.
- Vorher geladene `wuaueng.dll` Instanzen werden auch gepatcht.

### Diverse Verbesserungen seit meiner Batchfile Methode:

- **Es werden keine Systemdateien mehr verändert!**
- Heuristisches patchen, wodurch der Patch auch nach updates funktionieren sollte.
- Es wird C benutzt!
- Keine externen Abhängigkeiten.
