include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src)

#------------------------------------------------------------------------------
# STDIO Adapter Internal tests
#------------------------------------------------------------------------------
add_executable(stdio_adapter_mapper_test stdio_adapter_mapper_test.cpp ${ADAPTER_COMMON})
target_link_libraries(stdio_adapter_mapper_test stdc++fs hermes_stdio)
add_dependencies(stdio_adapter_mapper_test hermes_stdio)
pytest(stdio test_hermes_stdio_mapper)

#------------------------------------------------------------------------------
# STDIO Adapter End to End tests
#------------------------------------------------------------------------------
add_executable(stdio_adapter_test stdio_adapter_test.cpp ${ADAPTER_COMMON})
target_link_libraries(stdio_adapter_test hermes)
pytest(stdio test_stdio_basic)

add_executable(stdio_adapter_mpi_test stdio_adapter_mpi_test.cpp ${ADAPTER_COMMON})
# pytest(stdio test_stdio_basic_mpi 2 "")

add_executable(hermes_stdio_adapter_test stdio_adapter_test.cpp ${ADAPTER_COMMON})
target_link_libraries(hermes_stdio_adapter_test hermes_stdio)
add_dependencies(hermes_stdio_adapter_test hermes_stdio)
set_target_properties(hermes_stdio_adapter_test PROPERTIES COMPILE_FLAGS "-DHERMES_INTERCEPT=1")
pytest(stdio test_hermes_stdio_basic_small)
pytest(stdio test_hermes_stdio_basic_large)

add_executable(hermes_stdio_low_buf_adapter_test stdio_adapter_low_buffer_space_test.cpp ${ADAPTER_COMMON})
target_link_libraries(hermes_stdio_low_buf_adapter_test hermes_stdio)
add_dependencies(hermes_stdio_low_buf_adapter_test hermes_stdio)
set_target_properties(hermes_stdio_low_buf_adapter_test PROPERTIES COMPILE_FLAGS "-DHERMES_INTERCEPT=1")
pytest(stdio test_hermes_stdio_low_buf)

add_executable(hermes_stdio_adapter_mode_test stdio_adapter_mode_test.cpp ${ADAPTER_COMMON})
target_link_libraries(hermes_stdio_adapter_mode_test hermes_stdio)
add_dependencies(hermes_stdio_adapter_mode_test hermes_stdio)
set_target_properties(hermes_stdio_adapter_mode_test PROPERTIES COMPILE_FLAGS "-DHERMES_INTERCEPT=1")
pytest(stdio test_hermes_stdio_bypass)
pytest(stdio test_hermes_stdio_default)
pytest(stdio test_hermes_stdio_scratch)

add_executable(hermes_stdio_adapter_mpi_test stdio_adapter_mpi_test.cpp ${ADAPTER_COMMON})
target_link_libraries(hermes_stdio_adapter_mpi_test hermes_stdio)
add_dependencies(hermes_stdio_adapter_mpi_test hermes_stdio)
set_target_properties(hermes_stdio_adapter_mpi_test PROPERTIES COMPILE_FLAGS "-DHERMES_INTERCEPT=1")
pytest(stdio test_hermes_stdio_mpi_small)
pytest(stdio test_hermes_stdio_mpi_large)

set(STDIO_TESTS
  stdio_adapter_mapper_test
  stdio_adapter_test
  hermes_stdio_adapter_test
  hermes_stdio_low_buf_adapter_test
  hermes_stdio_adapter_mode_test
  stdio_adapter_mpi_test
  hermes_stdio_adapter_mpi_test
)

foreach(program ${STDIO_TESTS})
  target_include_directories(${program} PRIVATE ${HERMES_ADAPTER_DIR})
  target_include_directories(${program} PRIVATE ${HERMES_ADAPTER_TEST_DIR})
  target_link_libraries(${program} Catch2::Catch2 -lstdc++fs -lc MPI::MPI_CXX)
endforeach()

if(HERMES_INSTALL_TESTS)
    foreach(program ${STDIO_TESTS})
        install(
                TARGETS
                ${program}
                LIBRARY DESTINATION ${HERMES_INSTALL_LIB_DIR}
                ARCHIVE DESTINATION ${HERMES_INSTALL_LIB_DIR}
                RUNTIME DESTINATION ${HERMES_INSTALL_BIN_DIR}
        )
    endforeach()
endif()
