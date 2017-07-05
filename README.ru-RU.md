# wufuc [![](https://ci.appveyor.com/api/projects/status/0s2unkpokttyslf0?svg=true)](https://ci.appveyor.com/project/zeffy/wufuc) [![Click here to tip wufuc on Gratipay!](https://img.shields.io/gratipay/team/wufuc.svg)](https://gratipay.com/wufuc/)

[English](README.md) | **русский** | [Français](README.fr-FR.md) | [Deutsch](README.de-DE.md) | [Magyar](README.hu-HU.md)

[![Нажмите сюда, чтоюы поддержать wufuc пожертвованием на pledgie.com !](https://pledgie.com/campaigns/34055.png)](https://pledgie.com/campaigns/34055)

Отключает сообщение "Оборудование не поддерживается" в Windows Update, и позволяет продолжать устанавливать обновления на системах Windows 7 и 8.1 с процессорами Intel Kaby Lake, AMD Ryzen, и другими не поддерживаемыми.

## Загрузки [![](https://img.shields.io/github/downloads/zeffy/wufuc/total.svg)](../../releases)

### Последний стабильный релиз можно скачать [здесь](../../releases/latest)!

Храбрецы могут попробовать последний нестабильный билд [отсюда](https://ci.appveyor.com/project/zeffy/wufuc). **Использовать на свой собственный страх и риск!**

## Спонсоры

### [Advanced Installer](http://www.advancedinstaller.com/)

Для создания установщиков используется Advanced Installer по лицензии с открытым исходным кодом. Интуитивно понятный и удобный пользовательский интерфейс Advanced Installer'a позволяет быстро создать полнофункциональный инсталлятор с минимальными усилиями. [Проверьте сами!](http://www.advancedinstaller.com/) 

## Как сообщить об ошибке [![](https://isitmaintained.com/badge/resolution/zeffy/wufuc.svg)](https://isitmaintained.com/project/zeffy/wufuc)

Смотри в [CONTRIBUTING.ru-RU.md](CONTRIBUTING.ru-RU.md).

## Предисловие

Список изменений для обновлений Windows KB4012218 и KB4012219 включает следующее:

> Включено распознавание поддержки поколения процессоров и оборудования когда ПК пытается скачать обновления через Windows Update.

Эти обновления знаменуют [смену политики](https://blogs.windows.com/windowsexperience/2016/01/15/windows-10-embracing-silicon-innovation/), анонсированную некоторое время назад, где Microsoft объявила, что больше не будет поддерживать Windows 7 или 8.1 для следующих поколений процессоров Intel, AMD и Qualcomm.

По существу, это то же, что показать средний палец всем, кто решит не "обновляться" до вонючей кучи :shit: , известной как Windows 10, особенно учитывая то, что расширенный период поддержки для Windows 7 не закончится до 4 января 2020 и для Windows 8.1 - до 10 января 2023.

Это затронуло даже людей с более старыми процессорами Intel и AMD! Я видел сообщения от пользователей [Intel Atom Z530](../../issues/7), [Intel Core i5-M 560](../../issues/23), [Intel Core i5-4300M](../../issues/24), [Intel Atom D525](../../issues/34), [Intel Pentium B940](../../issues/63), и [AMD FX-8350](../../issues/32) - на всех них было заблокировано получение обновлений.

## Плохая Microsoft!

Если вам интересно, можете прочитать мою оригинальную записку об обнаружении проверки на тип процессора [тут](../../tree/old-kb4012218-19).

## Как работает этот патчер

Вкратце, в файле под названием `wuaueng.dll` есть 2 функции: [`IsDeviceServiceable(void)`](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185) и [`IsCPUSupported(void)`](https://gist.github.com/zeffy/1a8f8984d2bec97ae24af63a76278694). `IsDeviceServiceable(void)` по сути просто обертка над `IsCPUSupported(void)`, которая кэширует полученный результат и переиспользует его при последующих вызовах. 

Мой патчер пользуется этим, устанавливая флаг "первый запуск" в `FALSE` и кэшированный результат в `TRUE`.

- При старте системы назначенное задание wufuc запускается от имени пользователя `NT AUTHORITY\SYSTEM`.
- `wufuc` определяет группу служб, под которой выполняется процесс Windows Update (обычно `netsvcs`), и внедряется в неё.
- После внедрения применяется перехват `LoadLibraryEx`, который автоматчиески патчит `wuaueng.dll` при загрузке.
- Любая загруженная до этого `wuaueng.dll` тоже патчится.

### Несколько преимуществ перед методом batch-файла:

- **Нет модификаций в системных файлах!**
- Эвристический патчер - продолжит работать (я надеюсь) даже после выхода новых обновлений.
- C - лучший язык!
- Нет внешних зависимостей.
