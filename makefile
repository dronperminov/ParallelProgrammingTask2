COMPILER=mpixlC
FLAGS=-O3 -pedantic -Wall -std=c++11
FILES=ArgumentParser.cpp GraphGenerator.cpp GraphFiller.cpp CommunicationGenerator.cpp VectorMath.cpp ConjugateGradientSolver.cpp

all: main

main:
	$(COMPILER) $(FLAGS) $(FILES) main.cpp -o main
