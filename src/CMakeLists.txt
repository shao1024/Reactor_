# 将当前目录下的所有源文件添加到 SRC_LIST 变量中
aux_source_directory(./ SRC_LIST)

add_library(tcpserver SHARED 
        ${SRC_LIST})
target_link_libraries(tcpserver)

