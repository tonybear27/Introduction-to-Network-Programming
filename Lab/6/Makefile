
all: server.cpp client.cpp
	clear
	g++ -static -o server server.cpp
	g++ -static -o client client.cpp
	TOKEN=bDNg6gnfp6Ng3HPA ./submit.py ./server ./client

mem: srv_mem.cpp cli_mem.cpp
	clear
	g++ -static -o srv srv_mem.cpp
	g++ -static -o cli cli_mem.cpp
	python3 submit.py ./srv ./cli


.PHONY: clean

clean:
	rm *.out