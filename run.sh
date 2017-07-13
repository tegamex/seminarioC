files="$(find ./src -name '*.cpp')"
g++ $files -O3 -lm -lboost_system -lboost_chrono -lpthread -lX11 -I./thirdParty ./thirdParty/dlib/all/source.cpp -o ejecutable && echo 'Done' && ./ejecutable