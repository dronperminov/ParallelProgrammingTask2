COMPILER=mpicxx
FLAGS=-O3 -pedantic -Wall -std=c++11
FILES=ArgumentParser.cpp GraphGenerator.cpp GraphFiller.cpp CommunicationGenerator.cpp

all: main execute

main:
	$(COMPILER) $(FLAGS) $(FILES) main.cpp -o main

execute:
	rm output.txt -f && mpisubmit.pl -p 4 --stdout=output.txt --stderr=/dev/null ./main 4 4 3 4 2 2 1e-5 f
