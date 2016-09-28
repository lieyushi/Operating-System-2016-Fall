all: client server

client: main1.cpp
	g++ $< -o $@

server: server.cpp
	g++ $< -o $@

clean:
	rm client server