#gcc -std=c11 main.c -oquick && ./quick
#g++ -std=c++20 Contrib/Render.cpp Source/Main.cpp Source/Sprite.cpp -DUL_LINUX=1 -I/usr/include/SDL2 -IContrib -ISDL2 -g -lpthread -lGL -lSDL2 -o ul #&& ./ul
g++ -std=c++20 single_file.cpp -DUL_LINUX=1 -I/usr/include/SDL2 -IContrib -ISDL2 -g -lpthread -lGL -lSDL2 -o ul && ./ul
