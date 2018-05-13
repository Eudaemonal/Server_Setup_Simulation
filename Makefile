all:
	g++ -std=c++14 wrapper.cpp simulation.cpp -o wrapper
	g++ -std=c++14 design.cpp simulation.cpp -o design -lGLU -lGL -lglut 
	g++ -std=c++14 verification.cpp simulation.cpp -o verification -lGLU -lGL -lglut 