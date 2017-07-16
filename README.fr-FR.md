# wufuc [![](https://ci.appveyor.com/api/projects/status/0s2unkpokttyslf0?svg=true)](https://ci.appveyor.com/project/zeffy/wufuc)

[English](README.md) | [русский](README.ru-RU.md) | **Français** | [Deutsch](README.de-DE.md) | [Magyar](README.hu-HU.md) | [Portuguese (Brazil)](README.pt-BR.md)

[![Cliquez ici pour témoigner votre support à wufuc et faire une donation sur pledgie.com !](https://pledgie.com/campaigns/34055.png)](https://pledgie.com/campaigns/34055) <a href='https://gratipay.com/wufuc/'><img height=37 alt='Cliquez pour laisser un pourboire à wufuc sur Gratipay!' src='https://raw.githubusercontent.com/Cam/gratipay-badge/master/dist/gratipay.png' /></a>

Ce projet désactive le message de popup "Unsupported Hardware" pendant les mises à jour Windows, et permet de continuer à installer des updates sur les systèmes Windows 7 et 8.1, équipés de processeurs Intel Kaby Lake, AMD Ryzen, ou tout autre processor non supporté.

## Téléchargements [![](https://img.shields.io/github/downloads/zeffy/wufuc/total.svg)](../../releases)

### Vous pouvez obtenir la dernière version stable [ici](../../releases/latest) !

Si vous vous sentez courageux, vous pouvez essayer le dernier build instable [ici](https://ci.appveyor.com/project/zeffy/wufuc). **À utiliser à vos propres risques !**

## Sponsors

### [Advanced Installer](http://www.advancedinstaller.com/)

Les packages d'installation ont été créés avec Advanced Installer avec une license open source. L'interface utilisateur intuitive d'Advanced Installer m'a permis de créer un installeur complet avec un minimum d'effort. [Plus de détails ici !](http://www.advancedinstaller.com/)

## Reporter un problème [![](https://isitmaintained.com/badge/resolution/zeffy/wufuc.svg)](https://isitmaintained.com/project/zeffy/wufuc)

Voir [CONTRIBUTING.fr-FR.md](CONTRIBUTING.fr-FR.md).

## Préface

Le changelog des mises à jour Windows KB4012218 and KB4012219 incluait le message suivant:

> Activation de la détection de la génération de processeur et du support matériel quand le PC essaie de scanner ou télécharger grâce à Windows Update.

Ces updates ont marqué l'implémentation d'un [changement de politique](https://blogs.windows.com/windowsexperience/2016/01/15/windows-10-embracing-silicon-innovation/) qu'ils avaient annoncé quelques temps auparavant, et dans lequel Microsoft énonçait qu'ils ne supporteraient plus Windows 7 et 8.1 pour les nouvelles générations de processeurs Intel, AMD et Qualcomm.

C'est un majestueux doigt d'honneur à tous ceux qui ont décidé de ne pas "upgrader" vers la bouse connue sous le nom de Windows 10, en particulier en considérant que le support étendu de Windows 7 et Windows 8.1 ne se terminera pas avant le 4 janvier 2020 et 10 janvier 2023 respectivement.

Cela affecte également des gens avec des processeurs Intel et AMD plus vieux ! J'ai reçu des rapports d'utilisateurs pour [Intel Atom Z530](../../issues/7), [Intel Core i5-M 560](../../issues/23), [Intel Core i5-4300M](../../issues/24), [Intel Atom D525](../../issues/34), [Intel Pentium B940](../../issues/63), [AMD FX-8350](../../issues/32), et [AMD Turion 64 Mobile Technology ML-34](../../issues/80), tous empêchés de recevoir des updates.

## Méchant Microsoft !

Si vous êtes intéressés, vous pouvez lire mon écrit originel sur la découverte du check de CPU [ici (en anglais)](../../tree/old-kb4012218-19).

## Comment ça marche

De manière basique, dans le fichier `wuaueng.dll` il y a deux fonctions [`IsDeviceServiceable(void)`](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185) et [`IsCPUSupported(void)`](https://gist.github.com/zeffy/1a8f8984d2bec97ae24af63a76278694). `IsDeviceServiceable(void)` est un wrapper au dessus de `IsCPUSupported(void)` qui met en cache le résultat qu'il reçoit et le réutilise pour les appels suivants.

Mon patch tire avantage de comportement à mettre en cache le résultat en assignant la valeur de la "première exécution" à `FALSE` et le résultat en cache à `TRUE`.

- Au boot du système, la tâche planifiée wufuc s'exécute en tant qu'utilsateur `NT AUTHORITY\SYSTEM`
- `wufuc` détermine dans quel groupe de processus hôte le service Windows Update est exécuté (typiquement `netsvcs`), et s'injecte lui-même à l'intérieur.
- Une fois injecté, il applique un hook à `LoadLibraryEx` qui patche `wuaueng.dll` automatiquement à la volée quand il est chargé.
- Toute librairie `wuaueng.dll` précédemment chargée est aussi patchée.

### Plusieurs améliorations de mes méthodes par script batch :

- **Aucun fichier système n'est modifié !**
- Patch avec une base heuristique, ce qui signifie que cela devrait continuer de fonctionner même si d'autres updates sortent.
- Le langage C était le plus adapté.		
- Pas de dépendances externes.
