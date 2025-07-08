aux_source_directory(. SRC_LIST)
aux_source_directory(../../src/server/db DB_LIST)

set(TARGET_BIN_DIR "${CMAKE_SOURCE_DIR}/bin")  # 目标目录：/home/li/code/bin

add_executable(Pool ${SRC_LIST} ${DB_LIST})

target_link_libraries(Pool muduo_net muduo_base mysqlclient pthread)