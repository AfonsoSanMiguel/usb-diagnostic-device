echo "DIAGNOSTICO DE CPU"
echo "Data: $(date '+%Y-%m-%d %H:%M:%S')"

host="$HOSTNAME"
[ -z "$host" ] && [ -r /proc/sys/kernel/hostname ] && host=$(cat /proc/sys/kernel/hostname)
[ -z "$host" ] && host="indisponivel"
echo "Host: $host"
echo ""

modelo="indisponivel"
ncores="indisponivel"
if [ -r /proc/cpuinfo ]; then
    modelo=$(grep -m1 -i "model name" /proc/cpuinfo | cut -d: -f2- | sed 's/^ *//')
    [ -z "$modelo" ] && modelo=$(grep -m1 -i "^Hardware" /proc/cpuinfo | cut -d: -f2- | sed 's/^ *//')
    [ -z "$modelo" ] && modelo=$(grep -m1 -i "^Processor" /proc/cpuinfo | cut -d: -f2- | sed 's/^ *//')
    [ -z "$modelo" ] && modelo="indisponivel"
    ncores=$(grep -c "^processor" /proc/cpuinfo)
    [ "$ncores" -eq 0 ] 2>/dev/null && ncores="indisponivel"
fi
echo "Modelo   : $modelo"
echo "Cores    : $ncores"
echo ""

if [ -r /proc/stat ]; then
    read -r _ u1 n1 s1 i1 w1 q1 sq1 st1 _ < /proc/stat
    idle1=$((i1 + w1))
    total1=$((u1 + n1 + s1 + i1 + w1 + q1 + sq1 + st1))
    sleep 1
    read -r _ u2 n2 s2 i2 w2 q2 sq2 st2 _ < /proc/stat
    idle2=$((i2 + w2))
    total2=$((u2 + n2 + s2 + i2 + w2 + q2 + sq2 + st2))
    dtotal=$((total2 - total1))
    didle=$((idle2 - idle1))
    if [ "$dtotal" -gt 0 ]; then
        uso=$(awk "BEGIN { printf \"%.1f\", (($dtotal - $didle) / $dtotal) * 100 }")
        echo "Utilizacao CPU : ${uso}%"
    else
        echo "Utilizacao CPU : indisponivel"
    fi
else
    echo "Utilizacao CPU : indisponivel"
fi
echo ""

if [ -r /proc/loadavg ]; then
    read -r l1 l5 l15 _ < /proc/loadavg
    echo "Load average   : $l1 (1min)  $l5 (5min)  $l15 (15min)"
else
    echo "Load average   : indisponivel"
fi
echo ""

temp_file="/sys/class/thermal/thermal_zone0/temp"
if [ -r "$temp_file" ]; then
    raw=$(cat "$temp_file" 2>/dev/null)
    if [ -n "$raw" ] && [ "$raw" -eq "$raw" ] 2>/dev/null; then
        tempc=$(awk "BEGIN { printf \"%.1f\", $raw / 1000 }")
        echo "Temperatura    : ${tempc} C"
    else
        echo "Temperatura    : indisponivel"
    fi
else
    echo "Temperatura    : indisponivel"
fi
echo ""
echo "FIM"
