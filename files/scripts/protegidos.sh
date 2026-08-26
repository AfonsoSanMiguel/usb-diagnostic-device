#!/bin/bash

echo "DIAGNOSTICO DE FICHEIROS PROTEGIDOS"
echo "Data: $(date '+%Y-%m-%d %H:%M:%S')"
echo "Host: $(hostname)"
echo

check() {
    local f="$1" exp="$2"
    if [[ -e "$f" ]]; then
        local perm owner
        perm=$(stat -c '%a' "$f" 2>/dev/null)
        owner=$(stat -c '%U:%G' "$f" 2>/dev/null)
        printf '%-26s perm=%-4s dono=%-14s' "$f" "$perm" "$owner"
        if [[ -n "$exp" && "$perm" != "$exp" ]]; then
            printf '  [!] esperado %s\n' "$exp"
        else
            printf '  [ok]\n'
        fi
    else
        printf '%-26s inexistente\n' "$f"
    fi
}

echo "== Permissoes de ficheiros de configuracao sensiveis =="
check /etc/sudoers            440
check /etc/ssh/sshd_config    600
check /etc/shadow             640
check /etc/passwd             644
check /etc/group              644

echo
echo "FIM"
