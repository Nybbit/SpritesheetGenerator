# CMakeLists.txt

cmake_minimum_required(VERSION 3.1)
project(SpritesheetGeneratorExample)

add_executable(example Main.cpp ../Spritesheet.h ../Spritesheet.cpp ../lodepng/lodepng.h ../lodepng/lodepng.cpp)

set_target_properties(example PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED TRUE CXX_EXTENSIONS FALSE)
target_link_libraries(example PRIVATE "-lstdc++fs")
