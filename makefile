.PHONY:all
all:parser server
parser:parser.cc
	g++ -o $@ $^ -lboost_system -lboost_filesystem -std=c++11
server:server.cc
	g++ -o $@ $^ -ljsoncpp -std=c++11
.PHONY:clean
clean:
	rm -rf parser server
