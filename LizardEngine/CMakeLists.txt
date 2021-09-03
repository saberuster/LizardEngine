message("-- Link LizardEngine")

file(GLOB_RECURSE SRC_CODE ./Source/*.*)

add_library(LizardEngine ${SRC_CODE})

target_link_libraries(LizardEngine PRIVATE "d3d12.lib" "dxgi.lib" "d3dcompiler.lib" "dxguid.lib")