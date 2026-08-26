echo "DIAGNOSTICO DE PROCESSOS"
echo "Data: $(date '+%Y-%m-%d %H:%M:%S')"

host="$HOSTNAME"
[ -z "$host" ] && [ -r /proc/sys/kernel/hostname ] && host=$(cat /proc/sys/kernel/hostname)
[ -z "$host" ] && host="indisponivel"
echo "Host: $host"
echo ""

total=$(ls -d /proc/[0-9]* 2>/dev/null | wc -l)
if [ "$total" -gt 0 ] 2>/dev/null; then
    echo "Total de processos : $total"
else
    echo "Total de processos : indisponivel"
fi
echo ""

if command -v ps >/dev/null 2>&1; then
    dados=$(ps -eo pid,user,pcpu,pmem,comm 2>/dev/null | tail -n +2)

    if [ -n "$dados" ]; then
        echo "--- Top 5 por CPU (%) ---"
        printf "%-8s %-10s %6s %6s  %s\n" "PID" "UTILIZ." "CPU%" "MEM%" "NOME"
        printf "%s\n" "$dados" | sort -k3 -nr | head -5 | \
            awk '{ printf "%-8s %-10s %6s %6s  %s\n", $1, $2, $3, $4, $5 }'
        echo ""

        echo "--- Top 5 por memoria (%) ---"
        printf "%-8s %-10s %6s %6s  %s\n" "PID" "UTILIZ." "CPU%" "MEM%" "NOME"
        printf "%s\n" "$dados" | sort -k4 -nr | head -5 | \
            awk '{ printf "%-8s %-10s %6s %6s  %s\n", $1, $2, $3, $4, $5 }'
    else
        echo "Detalhe por processo : indisponivel"
    fi
else
    echo "Detalhe por processo : indisponivel (ps ausente)"
fi
echo ""
echo "FIM"
