make clean
make
round=1
time=0.0
for i in {1..$round}
do
    ntime=$(echo `./all_pair 1` | awk '{print $1 }')
    time=`echo "$time + $ntime" | bc`
    # echo $ntime $time
done
echo brute_with_path
atime=`echo "scale=8;$time / 5" | bc`
echo $time $atime

time=0.0
for i in {1..$round}
do
    ntime=$(echo `./all_pair 2` | awk '{print $1 }')
    time=`echo "$time + $ntime" | bc`
    # echo $ntime $time
done
echo brute_no_path
atime=`echo "scale=8;$time / 5" | bc`
echo $time $atime

time=0.0
for i in {1..$round}
do
    ntime=$(echo `./all_pair 3` | awk '{print $1 }')
    time=`echo "$time + $ntime" | bc`
    # echo $ntime $time
done
echo warshall_with_path
atime=`echo "scale=8;$time / 5" | bc`
echo $time $atime

time=0.0
for i in {1..$round}
do
    ntime=$(echo `./all_pair 4` | awk '{print $1 }')
    time=`echo "$time + $ntime" | bc`
    # echo $ntime $time
done
echo warshall_no_path
atime=`echo "scale=8;$time / 5" | bc`
echo $time $atime

time=0.0
for i in {1..$round}
do
    ntime=$(echo `./all_pair 4` | awk '{print $1 }')
    time=`echo "$time + $ntime" | bc`
    # echo $ntime $time
done
echo segment_with_path
atime=`echo "scale=8;$time / 5" | bc`
echo $time $atime

time=0.0
for i in {1..$round}
do
    ntime=$(echo `./all_pair 4` | awk '{print $1 }')
    time=`echo "$time + $ntime" | bc`
    # echo $ntime $time
done
echo segment_no_path
atime=`echo "scale=8;$time / 5" | bc`
echo $time $atime

time=0.0
for i in {1..$round}
do
    ntime=$(echo `./all_pair 4` | awk '{print $1 }')
    time=`echo "$time + $ntime" | bc`
    # echo $ntime $time
done
echo rule_based
atime=`echo "scale=8;$time / 5" | bc`
echo $time $atime
