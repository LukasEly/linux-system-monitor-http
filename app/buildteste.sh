#!/bin/bash

# Remove o binário existente
rm /home/lukas/Documentos/cso/linuxdistro/buildroot-2024.05.2/output/target/usr/bin/system_collector

# Compila o código fonte
i686-linux-gcc system_collector.c -O2 -o system_collector

# Move o codigo fonte
mv system_collector /home/lukas/Documentos/cso/linuxdistro/buildroot-2024.05.2/output/target/usr/bin

