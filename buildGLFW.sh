unzip glfw-3.4.zip
mkdir ./glfw-3.4/build
cd ./glfw-3.4/build
cmake -S .. -B . -D GLFW_BUILD_X11=0
make
