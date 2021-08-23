
add_definitions("-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX")

add_definitions("/wd26812")

set(ZED_EXE Zed)

set(SRCS
    "Zed.cpp"
    "Zed.h"
)

set(LIBS

)

add_executable(${ZED_EXE}
    "${SRCS}"
)

target_link_libraries(${ZED_EXE} ${LIBS})
