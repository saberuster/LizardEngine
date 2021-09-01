message("-- Link LizardEngine")
add_definitions(-DLIZARD_BUILD_DLL)
file(GLOB_RECURSE SRC_CODE ./Source/*.*)

add_library(LizardEngine SHARED ${SRC_CODE})

target_include_directories(LizardEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source/LizardEngine)
target_include_directories(LizardEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source/LizardEngine/Runtime)
target_include_directories(LizardEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source/LizardEngine/Runtime/Common)

target_link_libraries(LizardEngine PUBLIC "d3d12.lib" "dxgi.lib" "d3dcompiler.lib")