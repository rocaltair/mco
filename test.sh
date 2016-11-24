PROXY_PORT=12000
TARGET_PORT=13000

function make_all()
{
	make -C src
	make -C datetime
	make -C proxy
	make -C sleep
}

function make_cleanall()
{
	make clean -C src 
	make clean -C datetime
	make clean -C proxy
	make clean -C sleep
}

make_all
./proxy/forward $PROXY_PORT localhost $TARGET_PORT &
forward_pid=$!

./datetime/datetime $TARGET_PORT &
datetime_pid=$!

for i in $(seq 1 5); do
	sleep 1
	nc localhost $PROXY_PORT
done

echo kill $forward_pid $datetime_pid
kill $forward_pid $datetime_pid

./sleep/sleep

make_cleanall

