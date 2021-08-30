
add_definitions(-DLIZARD_PLATFORM_WINDOWS -DLIZARD_BUILD_DLL)
file(GLOB_RECURSE SRC_CODE ./Source/*.*)

add_library(LizardEngine SHARED ${SRC_CODE})

target_include_directories(LizardEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)