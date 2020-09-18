#gcc -g ./src/*.c -Iinclude -Iinclude/lib glad/src/glad.c glfw/build/src/libglfw3.a -Iglfw/include -Iglad/include -lgdi32 -o lpsx
gcc -O3 -s -flto -march=westmere -mtune=haswell ./src/*.c -Iinclude -Iinclude/lib glad/src/glad.c glfw/build/src/libglfw3.a -Iglfw/include -Iglad/include -lgdi32 -static -static-libgcc -o lpsx
