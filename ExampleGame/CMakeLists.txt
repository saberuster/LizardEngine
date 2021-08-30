
add_definitions(-DWINDOWS)

file(GLOB_RECURSE SRC_CODE ./Source/*.*)
add_executable(ExampleGame WIN32 ${SRC_CODE})
target_link_libraries(ExampleGame LINK_PUBLIC LizardEngine)
target_link_libraries(ExampleGame PRIVATE "d3d12.lib" "dxgi.lib" "d3dcompiler.lib")