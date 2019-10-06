if [ $# -ne 1 ]; then
    echo "Usage: $0 <server hostname>"
    echo "Eg. $0 emperor-07"
    exit 1
fi 
server=$1
SERVER_IP=$(host $server | head -1 | cut -d" " -f4)

make clean && make all

#./prec
#./syscall

#./pipe_latency
#./pipe_throughput

# Run UDP server on localhost in background
#./udp/udp_server_latency &
#./udp/udp_client_latency 127.0.0.1

./udp/udp_server_throughput &
./udp/udp_client_throughput 127.0.0.1

# Run UDP server on another host
#ssh $server nohup /u/n/i/nisargs/private/cs736/udp/udp_server_latency < /dev/null &
#./udp/udp_client_latency $SERVER_IP

ssh $server nohup /u/n/i/nisargs/private/cs736/udp/udp_server_throughput < /dev/null &
sleep 1
./udp/udp_client_throughput $SERVER_IP

exit 1

# Run TCP server on localhost in background
./tcp/tcp_server_latency &
./tcp/tcp_client_latency 127.0.0.1

./tcp/tcp_server_throughput &
./tcp/tcp_client_throughput 127.0.0.1

# Run TCP server on another host
ssh $server nohup /u/n/i/nisargs/private/cs736/tcp/tcp_server_latency < /dev/null &
./tcp/tcp_client_latency $SERVER_IP

ssh $server nohup /u/n/i/nisargs/private/cs736/tcp/tcp_server_throughput < /dev/null &
./tcp/tcp_client_throughput $SERVER_IP
