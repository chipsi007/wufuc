# wufuc [![](https://ci.appveyor.com/api/projects/status/0s2unkpokttyslf0?svg=true)](https://ci.appveyor.com/project/zeffy/wufuc)

[English](README.md) | [русский](README.ru-RU.md) | [Français](README.fr-FR.md) | [Deutsch](README.de-DE.md) | [Magyar](README.hu-HU.md) | **Portuguese (Brazil)**

[![Click here to lend your support to wufuc and make a donation at pledgie.com !](https://pledgie.com/campaigns/34055.png)](https://pledgie.com/campaigns/34055) <a href='https://gratipay.com/wufuc/'><img height=37 alt='Click here to tip wufuc on Gratipay!' src='https://raw.githubusercontent.com/Cam/gratipay-badge/master/dist/gratipay.png' /></a>

Desabilita a mensagem “Seu PC utiliza um processador que não é suportado por esta versão do Windows e você não receberá atualizações” do Windows Update, e permite que você continue instalando atualizações nos sistemas Windows 7 and 8.1 com os novos processadores Intel Kaby Lake, AMD Ryzen, ou outros processadores.

## Downloads [![](https://img.shields.io/github/downloads/zeffy/wufuc/total.svg)](../../releases)

### Você pode baixar a última versão estável [aqui](../../releases/latest)!

Se você é corajoso, você pode testar a último versão instável  [aqui](https://ci.appveyor.com/project/zeffy/wufuc). **Utilize-o por risco próprio**

## Patrocinadores

### [Advanced Installer](http://www.advancedinstaller.com/)
O instalador dos pacotes foram criados com Advanced Installer com a licença de cógido aberto. O Advanced Installer tem uma interface intuitiva e amigável que me permitiu a criar rapidamente um instalador completo com facilidade. [Dê uma olhada nisso](http://www.advancedinstaller.com/)

## Reportando problemas [![](https://isitmaintained.com/badge/resolution/zeffy/wufuc.svg)](https://isitmaintained.com/project/zeffy/wufuc)

Leia [CONTRIBUTING.pt-BR.md](CONTRIBUTING.pt-BR.md).

## Prefácio

O changelog para atualizações do Windows KB4012218 e KB4012219 incluindo o:

> Habilita detecção de geração de processador e suporte de hardware quando PC procura ou baixa atualizações pelo Windows Update.

Essas atualizações marcaram a implementação da [policy change](https://blogs.windows.com/windowsexperience/2016/01/15/windows-10-embracing-silicon-innovation/) que foi anunciado há um tempo atrás, onde a Microsoft declara que não irá fornecer suporte do Windows 7 ou 8.1 on na próxima geração de processadores Intel, AMD and Qualcomm.

Esse anúncio foi basicamente um foda-se para aqueles que decidiram não fazer upgrade para a grande "merda" conhecida por windows 10, especialmente considerando que o período de suporte para o Windows 7 e 8.1 só irá terminar em 4 de Janeiro de 2020 e 10 de Janeiro de 2023, respectivamente.

Essa atualização afetou até as pessoas que possuem processadores antigos da Intel e AMD! Alguns usuários já me relataram que tiveram o mesmo problema [Intel Atom Z530](../../issues/7), [Intel Core i5-M 560](../../issues/23), [Intel Core i5-4300M](../../issues/24), [Intel Atom D525](../../issues/34), [Intel Pentium B940](../../issues/63), [AMD FX-8350](../../issues/32), and [AMD Turion 64 Mobile Technology ML-34](../../issues/80) sendo todos bloqueados no Windows Update.

## Microsoft sacana!

Se você estiver interessado, você pode ler você pode ler as minhas primeiras anotações de como descrobri o CPU check [aqui](../../tree/old-kb4012218-19).

## Como funciona

Basicamente, dentro do arquivo chamado `wuaueng.dll` existem 2 funções: [`IsDeviceServiceable(void)`](https://gist.github.com/zeffy/e5ec266952932bc905eb0cbc6ed72185) and [`IsCPUSupported(void)`](https://gist.github.com/zeffy/1a8f8984d2bec97ae24af63a76278694). `IsDeviceServiceable(void)` é basicamente um desvio `IsCPUSupported(void)` que captura o resultado que ele recebe e a recicla nas próximas chamadas.

Meu patch altera o resultado do valor da "first run" de `FALSE` e transforma o resultado em `TRUE`.

- No boot do sistema a tarefa agendada do wufuc roda como usuário `NT AUTHORITY\SYSTEM`.
- `wufuc` determina que serviço host group processa o Windows Update roda (normalmente `netsvcs`), e se introduz nele.
- Uma vez introduzido, ele aplica o hook para `LoadLibraryEx` que automaticamente roda `wuaueng.dll` quando está carregado.
- Qualquer carregamento prévio do `wuaueng.dll` também é rodado. 

### Muitas melhorias no meu método batchfile:

- **Nenhum arquivo do sistema é alterado!**
- Heuristic-based patching, isso significa que ele funcionará mesmo com novas atualizações do Windows.
- C é a melhor linguagem!
- Não depende de nada externo.
