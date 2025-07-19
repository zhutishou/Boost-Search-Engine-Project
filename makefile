.PHONY:all
all:parser debug http_server
parser:parser.cc
	g++ -o $@ $^ -lboost_system -lboost_filesystem -std=c++11
# server:server.cc
# 	g++ -o $@ $^ -ljsoncpp -std=c++11
debug:debug.cc
	g++ -o $@ $^ -ljsoncpp -std=c++11
http_server:http_server.cc
	g++ -o $@ $^ -ljsoncpp -std=c++11 -lpthread
.PHONY:clean
clean:
	rm -rf parser debug http_server
