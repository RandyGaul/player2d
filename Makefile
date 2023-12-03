CXXFLAGS = -I. -std=c++11
LDFLAGS = -lSDL2 -ldl

player2d : main.cpp
	$(CXX) $(CXXFLAGS) main.cpp $(LDGLAGS) -o $@

.PHONY: clean player2d
clean :
	rm player2d
