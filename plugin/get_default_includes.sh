first=$(($(cpp -v /dev/null -o /dev/null 2>&1 | grep -n "#include <...> search starts here:" | grep -o "[0-9]*") + 1))
last=$(($(cpp -v /dev/null -o /dev/null 2>&1 | grep -n "End of search list" | grep -o "[0-9]*") - 1))

incs=""

i=$first
while [[ $i -le $last ]]
do
  inc_path=$(cpp -v /dev/null -o /dev/null 2>&1 | awk "NR==$i" | awk 'sub(/^.{1}/,"")')
  incs=$incs'::'$inc_path
  ((i = i + 1))
done
echo $incs
