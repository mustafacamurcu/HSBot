game: main
	python game.py

run: main
	./main

main: main.cpp
	g++ main.cpp -o main -framework CoreGraphics -framework CoreFoundation -framework ImageIO -framework CoreServices
