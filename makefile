COMPILER=g++
FLAGS=-O3 -fopenmp -pedantic -Wall
FILES=ArgumentParser.cpp GraphGenerator.cpp

all: main

main:
	$(COMPILER) $(FLAGS) $(FILES) main.cpp -o main && main 4 4 3 4 2 2 1e-5 f
