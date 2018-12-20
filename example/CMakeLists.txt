# CMakeLists.txt

cmake_minimum_required(VERSION 3.1)
project(SpritesheetGeneratorExample)

add_executable(example Main.cpp ../Spritesheet.h ../Spritesheet.cpp)
target_include_directories(example PRIVATE ThirdParty/stb)
target_include_directories(example PRIVATE ThirdParty/glm)

set_target_properties(example PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED TRUE CXX_EXTENSIONS FALSE)
target_link_libraries(example PRIVATE "-lstdc++fs")
