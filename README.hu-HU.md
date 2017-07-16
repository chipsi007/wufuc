# wufuc [![](https://ci.appveyor.com/api/projects/status/0s2unkpokttyslf0?svg=true)](https://ci.appveyor.com/project/zeffy/wufuc)

[English](README.md) | [русский](README.ru-RU.md) | [Français](README.fr-FR.md) | [Deutsch](README.de-DE.md) | **Magyar** | [Portuguese (Brazil)](README.pt-BR.md)

[![Kattints ide hogy támogassad a wufuc-ot a pledgie.com-on !](https://pledgie.com/campaigns/34055.png)](https://pledgie.com/campaigns/34055) <a href='https://gratipay.com/wufuc/'><img height=37 alt='Click here to tip wufuc on Gratipay!' src='https://raw.githubusercontent.com/Cam/gratipay-badge/master/dist/gratipay.png' /></a>

Kikapcsolja az "Windows Update Unsupported Hardware/Nem támogatott hardver" jelentését, és lehetővé teszi a frissítések telepítését Intel Kaby Lake, AMD Ryzen, vagy más nem támogatott processzoros Windows 7 és 8.1 rendszereken.

## Letöltések [![](https://img.shields.io/github/downloads/zeffy/wufuc/total.svg)](../../releases)

### A legfrissebb stabil verziót [itt](../../releases/latest) találod!

Ha bátornak érzed magad, itt próbálhatod ki a legfrissebb build-eket [here](https://ci.appveyor.com/project/zeffy/wufuc). **Saját felelősségre!**

## Szponzorok

### [Advanced Installer](http://www.advancedinstaller.com/)
A telepítő csomagok Advanced Installerel lettek elkészítve, open source licenc alatt. Az Advanced Installer program intuitív és barátságos UI-jával gyorsan és kényelmesen el tudtam készíteni a telepítőt. [Nézd meg!](http://www.advancedinstaller.com/)

## Hibajelentés [![](https://isitmaintained.com/badge/resolution/zeffy/wufuc.svg)](https://isitmaintained.com/project/zeffy/wufuc)

Lásd [CONTRIBUTING.hu-HU.md](CONTRIBUTING.hu-HU.md).

## Előszó

Az KB4012218 és KB4012219 kódszámú Windows frissítések leírása ezeket az információkat tartalmazta:

> Enabled detection of processor generation and hardware support when PC tries to scan or download updates through Windows Update.

Ez a frissítés lényegében egy [filozófiavĺtást](https://blogs.windows.com/windowsexperience/2016/01/15/windows-10-embracing-silicon-innovation/) jelentett, hogy a Microsoft nem fogja támogatni a Windows 7 vagy 8.1 következő generációs Intel, AMD és Qualcomm processzoron.

A szerző véleményéről evvel kapcsolatban legjobban az angol verzióból informálódhattok.

## Rossz Microsoft!

Ha érdekelnek a részletek, [itt](../../tree/old-kb4012218-19) olvashatsz tovább.

## Hogy működik?

A `wuaueng.dll` fájl két függvényt tartalmaz: [`IsDeviceServiceable(void)`](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185) és [`IsCPUSupported(void)`](https://gist.github.com/zeffy/1a8f8984d2bec97ae24af63a76278694). `IsDeviceServiceable(void)` egy egyszerű wrapper a `IsCPUSupported(void)` függvény körül, ami eltárolja amit olvas és új felhíváskor újra felhasználja ezeket.

A patch ezt a tárolást használja ki, és átállítja a "first run" értéket `FALSE`-ra és a cached result-t `TRUE`-ra.

- A rendszer bootolásakor a wufuc munkacsomag elindul az `NT AUTHORITY\SYSTEM` felhasználó alatt.
- `wufuc` megkeresi melyik service process-ben fut a Windows Update service (tipikusan `netsvcs`), és belekapcsolódik.
- Miután ez megtörtént, a `LoadLibraryEx` segítségével automatikusan módosítja a `wuaueng.dll`-t ha az be lesz töltve.
- Egy előzőlegesen betöltött `wuaueng.dll` is meg lesz patchelve.

### Fejlesztések a régi batchfájl módszerhez képest:

- **Rendszerfájlok nem lesznek módosítva**
- Heurisztikusan dolgozó patchelés, ami segítségével a program frissítések után is működik.
- C nyelv használata skriptelés helyett!
- Nincs külső függőség.
