add_definitions(-DWINDOWS)

file(GLOB_RECURSE SRC_CODE ./Source/*.*)

add_executable(ExampleGame WIN32 ${SRC_CODE})
target_link_libraries(ExampleGame PRIVATE LizardEngine)