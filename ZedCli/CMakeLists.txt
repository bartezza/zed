
add_definitions("-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX")

add_definitions("/wd26812")

set(ZED_EXE ZedCli)

set(SRCS
    "ZedCli.cpp"
)

set(LIBS

)

add_executable(${ZED_EXE}
    "${SRCS}"
)

target_link_libraries(${ZED_EXE} ${LIBS} Zed)
