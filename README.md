# Monitor de sistema Linux com servidor HTTP

Aplicação em C que lê informações do sistema em `/proc` e gera uma página HTML servida por um servidor HTTP mínimo, também em C. As duas aplicações rodam numa distribuição Linux gerada com **Buildroot** e executada no **QEMU**, e sobem automaticamente pelos scripts de inicialização.

## Contexto

Trabalho 1 da disciplina **Construção de Sistemas Operacionais** (PUCRS), 5º semestre de Engenharia de Computação.

## Autores

- Lukas Ely
- Bruno Becker Silva

## Tecnologias

- C (POSIX, sockets)
- Linux `/proc` (`version`, `uptime`, `stat`, `meminfo`, `diskstats`, `net/dev` e outros)
- Buildroot 2024.05 e QEMU
- Scripts de init em shell

## Informações coletadas

Versão do kernel, tempo ligado e ocioso, data e hora do RTC, modelo e uso da CPU, carga do sistema, uso de memória, operações de E/S, sistemas de arquivos suportados, dispositivos, interfaces de rede e lista de processos.

## Como compilar e executar

No host, para testar fora do Buildroot:

```sh
gcc -o system_collector app/system_collector.c
gcc -o simple_http_server app/simple_http_server.c
./system_collector &      # gera o index.html
./simple_http_server      # serve a página
```

O caminho do HTML gerado está na constante `HTML_FILE_PATH`, no início de `system_collector.c`. Ajuste para o seu ambiente.

No Buildroot:

1. Copie os scripts de `buildroot/` para a pasta `custom-scripts` do projeto e as aplicações para `apps`.
2. Configure `pre-build.sh` como script de pré-build. Ele instala `S41network-config` (rede entre host e QEMU) e `S50server` (inicia as duas aplicações).
3. Gere a imagem e inicie o QEMU com `qemu-ifup` para criar a interface de rede do host.
4. Acesse `http://192.168.1.10` a partir do host.

## Estrutura de pastas

```
.
├── app/
│   ├── system_collector.c     coleta os dados e gera o HTML
│   ├── simple_http_server.c   servidor HTTP mínimo
│   ├── index.html             página inicial de exemplo
│   └── buildteste.sh          compila e copia para o target do Buildroot
└── buildroot/
    ├── S41network-config      configuração de rede no boot
    ├── S50server              inicia as aplicações no boot
    ├── pre-build.sh           instala scripts e arquivos no target
    └── qemu-ifup              cria a interface de rede no host
```
