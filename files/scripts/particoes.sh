echo "DIAGNOSTICO DE PARTICOES / DISCO "
echo "Data: $(date '+%Y-%m-%d %H:%M:%S')"

host="$HOSTNAME"
[ -z "$host" ] && [ -r /proc/sys/kernel/hostname ] && host=$(cat /proc/sys/kernel/hostname)
[ -z "$host" ] && host="indisponivel"
echo "Host: $host"
echo ""

if ! command -v df >/dev/null 2>&1; then
    echo "Particoes: indisponivel"
    echo "FIM"
    exit 0
fi

saida=$(df -P 2>/dev/null | awk '
    function human(kb,   v, u) {
        v = kb; u = "KB"
        if (v >= 1048576)      { v = v / 1048576; u = "GB" }
        else if (v >= 1024)    { v = v / 1024;    u = "MB" }
        return sprintf("%.1f %s", v, u)
    }
    NR == 1 { next }
    $1 ~ /^\/dev\// {
        printf "Ponto de montagem : %s\n", $6
        printf "  Total     : %s\n", human($2)
        printf "  Usado     : %s (%s)\n", human($3), $5
        printf "  Disponivel: %s\n", human($4)
        printf "\n"
        encontrou = 1
    }
    END { if (!encontrou) print "SEM_PARTICOES_REAIS" }
')

if [ "$saida" = "SEM_PARTICOES_REAIS" ] || [ -z "$saida" ]; then
    echo "Nenhuma particao real detetada."
else
    printf "%s\n" "$saida"
fi

echo "FIM"
