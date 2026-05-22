set(NAME application)
add_executable(${NAME})

target_compile_features(${NAME} PUBLIC c_std_11)
target_compile_definitions(${NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${NAME} PUBLIC -Wall)
endif()

#-------------------------------------------------------------------------------
# Target sources

target_sources(${NAME} PRIVATE
    "src/main.c"
    "src/settings.h"

    "src/graph/graph.c"
    "src/graph/graph.h"
    "src/graph/shortest_path.c"
    "src/graph/shortest_path.h"
    "src/graph/tsp.c"
    "src/graph/tsp.h"

    "src/utils/list_int.c"
    "src/utils/list_int.h"
    "src/utils/tools.c"
    "src/utils/tools.h"
    "src/utils/heap_int.c"
    "src/utils/heap_int.h"
     "src/utils/geojson.c" 
     "src/utils/geojson.h" 
     "src/utils/search_trough.c" 
     "src/utils/search_trough.h")

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(${NAME} PUBLIC ${MATH_LIBRARY})
endif()

#-------------------------------------------------------------------------------
# Other include directories

target_include_directories(
    ${NAME} PUBLIC
    "src"
)
