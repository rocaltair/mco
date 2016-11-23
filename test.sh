make -C src
make -C datetime
make -C proxy

PROXY_PORT=12000
TARGET_PORT=13000

./proxy/forward $PROXY_PORT localhost $TARGET_PORT &
forward_pid=$!

./datetime/datetime $TARGET_PORT &
datetime_pid=$!

for i in $(seq 1 5); do
	sleep 1
	nc localhost $PROXY_PORT
done

echo $forward_pid $datetime_pid

kill $forward_pid $datetime_pid
