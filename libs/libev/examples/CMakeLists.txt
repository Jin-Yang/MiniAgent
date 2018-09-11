INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/libev)

ADD_EXECUTABLE(timer timer.c)
TARGET_LINK_LIBRARIES(timer ev m)

ADD_EXECUTABLE(periodic periodic.c)
TARGET_LINK_LIBRARIES(periodic ev m)

ADD_EXECUTABLE(httpd httpd.c common.c)
TARGET_LINK_LIBRARIES(httpd ev m)

ADD_EXECUTABLE(child child.c common.c)
TARGET_LINK_LIBRARIES(child ev m)

ADD_EXECUTABLE(signal signal.c common.c)
TARGET_LINK_LIBRARIES(signal ev m)
