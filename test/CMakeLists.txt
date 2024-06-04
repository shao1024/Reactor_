add_executable(tcpepoll tcpepoll.cpp)
target_link_libraries(tcpepoll
)

add_executable(tcpserver_v1 tcpserver_v1.cpp)
target_link_libraries(tcpserver_v1
                      tcpserver )