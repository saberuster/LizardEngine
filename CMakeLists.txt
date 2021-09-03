cmake_minimum_required(VERSION 3.0.0)
project(LizardEngine VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")

add_compile_options(
    "$<$<CXX_COMPILER_ID:MSVC>:/experimental:module>"
)


# 将 ExampleGame.exe 和 LizardEngine.dll 都放到 ./build/bin/ 下防止debug的时候链接不到
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory(LizardEngine)
add_subdirectory(ExampleGame)
