#!/bin/bash

echo "DIAGNOSTICO DE LOGS DO SISTEMA"
echo "Data: $(date '+%Y-%m-%d %H:%M:%S')"
echo "Host: $(hostname)"
echo

echo "== journalctl : avisos e erros (ultimas 40) =="
if command -v journalctl >/dev/null 2>&1; then
    journalctl -p warning..err -n 40 --no-pager 2>/dev/null || echo "indisponivel"
else
    echo "journalctl indisponivel"
fi
echo

echo "== dmesg : avisos e erros (ultimas 40) =="
if command -v dmesg >/dev/null 2>&1; then
    dmesg --level=warn,err 2>/dev/null | tail -40 \
        || dmesg 2>/dev/null | tail -40 \
        || echo "indisponivel"
else
    echo "dmesg indisponivel"
fi

echo
echo "FIM"
