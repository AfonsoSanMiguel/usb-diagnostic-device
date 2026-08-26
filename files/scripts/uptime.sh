echo "DIAGNOSTICO DE UPTIME"
echo "Data: $(date '+%Y-%m-%d %H:%M:%S' 2>/dev/null)"

host="$HOSTNAME"
[ -z "$host" ] && [ -r /proc/sys/kernel/hostname ] && host=$(cat /proc/sys/kernel/hostname)
[ -z "$host" ] && host="indisponivel"
echo "Host: $host"
echo ""

if [ -r /proc/uptime ]; then
    read -r up_seg _ < /proc/uptime
    up_int=${up_seg%.*}
    if [ -n "$up_int" ] && [ "$up_int" -ge 0 ] 2>/dev/null; then
        dias=$(( up_int / 86400 ))
        horas=$(( (up_int % 86400) / 3600 ))
        mins=$(( (up_int % 3600) / 60 ))
        echo "Uptime         : ${dias}d ${horas}h ${mins}min"
    else
        echo "Uptime         : indisponivel"
    fi
else
    echo "Uptime         : indisponivel"
fi
echo ""

if [ -r /proc/stat ]; then
    btime=$(grep '^btime' /proc/stat 2>/dev/null | awk '{print $2}')
    if [ -n "$btime" ]; then
        boot_str=$(date -d "@$btime" '+%Y-%m-%d %H:%M:%S' 2>/dev/null)
        [ -z "$boot_str" ] && boot_str="indisponivel"
        echo "Arranque       : $boot_str"
    else
        echo "Arranque       : indisponivel"
    fi
else
    echo "Arranque       : indisponivel"
fi
echo ""

if [ -r /proc/loadavg ]; then
    read -r l1 l5 l15 _ < /proc/loadavg
    echo "Load average   : $l1 (1min)  $l5 (5min)  $l15 (15min)"
else
    echo "Load average   : indisponivel"
fi
echo ""
echo "FIM"

exit 0
