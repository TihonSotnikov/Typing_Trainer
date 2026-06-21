
cmake -S. -Bbuild --fresh -DCMAKE_BUILD_TYPE=Release
cmake --build build --config release
Remove-Item -Recurse dist
cmake --install build --config release --prefix "$pwd/dist"