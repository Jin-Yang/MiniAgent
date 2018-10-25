
ADD_EXECUTABLE(testing testing.c)
ADD_TEST(NAME testing COMMAND ${WRAPPER_CMD} ./testing)

#ADD_EXECUTABLE(tping ping.c ev_mock.c)
#TARGET_LINK_LIBRARIES(tping libz.a)
#TARGET_COMPILE_OPTIONS(tping PRIVATE "-DPING_MOCK=2")
#ADD_TEST(NAME testing-library-ping COMMAND ${WRAPPER_CMD} ./tping2)
#SET_TESTS_PROPERTIES(testing-library-ping PROPERTIES FAIL_REGULAR_EXPRESSION "reachable")
