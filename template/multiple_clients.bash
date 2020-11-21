

echo Starting requests

for j in $(seq 1 6)
do
	for ((i=15;i>0;i-=1)) ;
	do
		./wclient localhost 8003 /files/test$i.html &
	done
done

echo Requests completed
