cmake_minimum_required (VERSION 3.15.0)

project (Kraps VERSION 0.0.1 DESCRIPTION "Some Description.")


file(GLOB KRAPS_SRC ${PROJECT_SOURCE_DIR}/dafx/*.cpp
	${PROJECT_SOURCE_DIR}/filters/*.cpp
	${PROJECT_SOURCE_DIR}/generators/*.cpp
	${PROJECT_SOURCE_DIR}/processor/*.cpp
	${PROJECT_SOURCE_DIR}/processor/midi/*.cpp
	${PROJECT_SOURCE_DIR}/misc/*.cpp
	${PROJECT_SOURCE_DIR}/modulators/*.cpp

)
   	   
file(GLOB KRAPS_HDR ${PROJECT_SOURCE_DIR}/dafx/*.hpp
	${PROJECT_SOURCE_DIR}/fft/*.hpp
	${PROJECT_SOURCE_DIR}/filters/*.hpp
	${PROJECT_SOURCE_DIR}/generators/*.hpp
	${PROJECT_SOURCE_DIR}/processor/*.hpp
	${PROJECT_SOURCE_DIR}/processor/midi/*.hpp
	${PROJECT_SOURCE_DIR}/misc/*.hpp
	${PROJECT_SOURCE_DIR}/modulators/*.hpp
	${PROJECT_SOURCE_DIR}/simd/*.hpp
)

file (GLOB_RECURSE KRAPS_SRC_SERIALIZE ${PROJECT_SOURCE_DIR}/serialize *.cpp)
file (GLOB_RECURSE KRAPS_HDR_SERIALIZE ${PROJECT_SOURCE_DIR}/serialize *.hpp)

file (GLOB_RECURSE KRAPS_SRC_PROCESSOR ${PROJECT_SOURCE_DIR}/processor *.cpp)
file (GLOB_RECURSE KRAPS_HDR_PROCESSOR ${PROJECT_SOURCE_DIR}/processor *.hpp)

set (ALL_SRC ${KRAPS_HDR} ${KRAPS_SRC} ${KRAPS_SRC_SERIALIZE} ${KRAPS_HDR_SERIALIZE} ${KRAPS_SRC_PROCESSOR} ${KRAPS_HDR_PROCESSOR})

add_compile_definitions(_​USE_MATH_DEFINES)
add_definitions(-D_USE_MATH_DEFINES)

add_library (${PROJECT_NAME} STATIC
        ${ALL_SRC})
		
target_include_directories (${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR})
	
target_compile_features (Kraps PRIVATE cxx_std_17)

