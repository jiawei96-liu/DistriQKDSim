
# 生成Web应用程序
set(OATPP_INCLUDE_DIR /usr/local/include/oatpp-1.3.0/oatpp)
add_executable(webapp
    main.cpp
    svc/NetService.cpp
    dao/SimDao.cpp
    Utils.cpp
)

find_package(Threads REQUIRED)
# 链接Alg模块和Oat++库
target_link_libraries(webapp
    PRIVATE
    alg
    /usr/local/lib/oatpp-1.3.0/liboatpp.a
    Threads::Threads
    /usr/lib/x86_64-linux-gnu/libmysqlcppconn.so
)

# 设置包含目录
target_include_directories(webapp PRIVATE ${CMAKE_SOURCE_DIR} ${OATPP_INCLUDE_DIR})
