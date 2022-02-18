LIBS	:= -lSDL2main -lSDL2
SOURCE	:= ComplexDynamics.cpp
FLAGS	:= -O3 -Wall

run:
	g++ $(FLAGS) $(SOURCE) $(LIBS)
	./a.out
	rm a.out
