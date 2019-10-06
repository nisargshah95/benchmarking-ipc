all : prec syscall pipe_latency pipe_throughput udp_server_latency udp_server_throughput udp_client_latency udp_client_throughput tcp_server_latency tcp_server_throughput tcp_client_latency tcp_client_throughput

prec :
	gcc prec.c -o prec

syscall :
	gcc syscall.c -o syscall

pipe_latency :
	gcc pipe_latency.c -o pipe_latency

pipe_throughput :
	gcc pipe_throughput.c -o pipe_throughput

udp_server_latency :
	gcc udp/udp_server_latency.c -o udp/udp_server_latency

udp_server_throughput :
	gcc udp/udp_server_throughput.c -o udp/udp_server_throughput

udp_client_latency :
	gcc udp/udp_client_latency.c -o udp/udp_client_latency

udp_client_throughput :
	gcc udp/udp_client_throughput.c -o udp/udp_client_throughput

tcp_server_latency :
	gcc tcp/tcp_server_latency.c -o tcp/tcp_server_latency

tcp_server_throughput :
	gcc tcp/tcp_server_throughput.c -o tcp/tcp_server_throughput

tcp_client_latency :
	gcc tcp/tcp_client_latency.c -o tcp/tcp_client_latency

tcp_client_throughput :
	gcc tcp/tcp_client_throughput.c -o tcp/tcp_client_throughput

clean :
	rm -f prec syscall pipe_latency pipe_throughput udp/udp_server_latency udp/udp_server_throughput udp/udp_client_latency udp/udp_client_throughput tcp/tcp_server_latency tcp/tcp_server_throughput tcp/tcp_client_latency tcp/tcp_client_throughput
