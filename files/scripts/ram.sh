echo "DIAGNOSTICO DE MEMORIA RAM"
echo "Data: $(date '+%Y-%m-%d %H:%M:%S')"

host="$HOSTNAME"
[ -z "$host" ] && [ -r /proc/sys/kernel/hostname ] && host=$(cat /proc/sys/kernel/hostname)
[ -z "$host" ] && host="indisponivel"
echo "Host: $host"
echo ""

if [ ! -r /proc/meminfo ]; then
    echo "Memoria: indisponivel"
    echo "FIM"
    exit 0
fi

get_meminfo() {
    grep -m1 "^$1:" /proc/meminfo | awk '{print $2}'
}

mem_total=$(get_meminfo MemTotal)
mem_free=$(get_meminfo MemFree)
mem_avail=$(get_meminfo MemAvailable)
swap_total=$(get_meminfo SwapTotal)
swap_free=$(get_meminfo SwapFree)

if [ -z "$mem_avail" ]; then
    buffers=$(get_meminfo Buffers)
    cached=$(get_meminfo Cached)
    [ -z "$buffers" ] && buffers=0
    [ -z "$cached" ] && cached=0
    [ -n "$mem_free" ] && mem_avail=$((mem_free + buffers + cached))
fi

if [ -n "$mem_total" ] && [ "$mem_total" -gt 0 ] 2>/dev/null; then
    total_mb=$(awk "BEGIN { printf \"%.0f\", $mem_total / 1024 }")
    echo "RAM total      : ${total_mb} MB"

    if [ -n "$mem_avail" ]; then
        usada_kb=$((mem_total - mem_avail))
        usada_mb=$(awk "BEGIN { printf \"%.0f\", $usada_kb / 1024 }")
        avail_mb=$(awk "BEGIN { printf \"%.0f\", $mem_avail / 1024 }")
        pct=$(awk "BEGIN { printf \"%.1f\", ($usada_kb / $mem_total) * 100 }")
        echo "RAM usada      : ${usada_mb} MB (${pct}%)"
        echo "RAM disponivel : ${avail_mb} MB"
    else
        echo "RAM usada      : indisponivel"
    fi

    if [ -n "$mem_free" ]; then
        free_mb=$(awk "BEGIN { printf \"%.0f\", $mem_free / 1024 }")
        echo "RAM livre      : ${free_mb} MB"
    fi
else
    echo "RAM total      : indisponivel"
fi
echo ""

if [ -n "$swap_total" ] && [ "$swap_total" -gt 0 ] 2>/dev/null; then
    swap_total_mb=$(awk "BEGIN { printf \"%.0f\", $swap_total / 1024 }")
    if [ -n "$swap_free" ]; then
        swap_usada_kb=$((swap_total - swap_free))
        swap_usada_mb=$(awk "BEGIN { printf \"%.0f\", $swap_usada_kb / 1024 }")
        spct=$(awk "BEGIN { printf \"%.1f\", ($swap_usada_kb / $swap_total) * 100 }")
        echo "Swap total     : ${swap_total_mb} MB"
        echo "Swap usada     : ${swap_usada_mb} MB (${spct}%)"
    else
        echo "Swap total     : ${swap_total_mb} MB"
    fi
else
    echo "Swap           : nao configurada"
fi
echo ""
echo "FIM"
