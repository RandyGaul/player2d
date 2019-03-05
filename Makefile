player2d : main.cpp
	g++ -I. -L. main.cpp -o $@ -ldl -lSDL2 -std=c++11