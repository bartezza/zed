﻿
add_definitions("-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX")

add_definitions("/wd26812")

set(ZED_LIB Zed)

set(SRCS
    "Zed.cpp"
    "Zed.h"
)

set(LIBS

)

add_library(${ZED_LIB}
    "${SRCS}"
)

# target_link_libraries(${ZED_EXE} ${LIBS})
