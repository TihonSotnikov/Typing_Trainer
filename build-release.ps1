cmake --preset vcpkg-release
cmake --build --preset release
if (Test-Path dist) { Remove-Item -Recurse -Force dist }
cmake --install build/vcpkg-release --prefix "$pwd/dist"
