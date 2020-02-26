
set(SOURCES simple_test.c)
add_executable(simple_test ${SOURCES})
target_link_libraries(simple_test soem)
install(TARGETS simple_test DESTINATION bin)

set(SOURCES simple_test2.c)
add_executable(simple_test2 ${SOURCES})
target_link_libraries(simple_test2 soem)
install(TARGETS simple_test2 DESTINATION bin)

set(SOURCES simple_test3.c)
add_executable(simple_test3 ${SOURCES})
target_link_libraries(simple_test3 soem)
install(TARGETS simple_test3 DESTINATION bin)
