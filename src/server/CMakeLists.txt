aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)#./db 表示 ​​当前 CMakeLists.txt 文件所在目录下的 db 子目录​​。
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./redis REDIS_LIST)

set(TARGET_BIN_DIR "${CMAKE_SOURCE_DIR}/bin")  # 目标目录：/home/li/code/bin

# 自己创建的配置文件目录
set(MYSQL_CONF_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/db/mysql.conf")

# 将配置文件复制到构建目录（与可执行文件同一目录）
configure_file(${MYSQL_CONF_SOURCE} ${TARGET_BIN_DIR}/mysql.conf COPYONLY)
#${CMAKE_CURRENT_BINARY_DIR}：CMakeLists.txt 文件对应的构建目录
#COPYONLY：仅复制

add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})

target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)