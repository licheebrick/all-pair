OUTPUT = ./exe/

main: main.o network.o router.o
	g++ -o ./exe/main main.o network.o router.o
main.o: router.h network.h main.cpp
	g++ -c main.cpp
network.o: router.h network.h network.cpp
	g++ -c network.cpp
router.o: router.h router.cpp
	g++ -c router.cpp
clean:
	rm -rf *.o