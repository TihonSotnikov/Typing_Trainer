
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build --config release
cmake --install build --config release --prefix "$pwd/dist"