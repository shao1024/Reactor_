# add_executable(tcpepoll tcpepoll.cpp)
# target_link_libraries(tcpepoll
# )

#add_executable(tcpserver_v1 tcpserver_v1.cpp)
#target_link_libraries(tcpserver_v1
#                      tcpserver )

add_executable(echoserver_v1 echoserver_v1.cpp)
target_link_libraries(echoserver_v1
                      tcpserver )