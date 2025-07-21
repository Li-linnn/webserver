set(SRC_LIST
    pool_final.cpp    
    #threadPool.cpp
    thread.cpp
    #main.cpp
    )

add_library(thread_pool ${SRC_LIST})
#add_executable(thread_pool ${SRC_LIST})

target_include_directories(
    thread_pool PUBLIC
    ${PROJECT_SOURCE_DIR}/test_threadPool/include
)

#set_target_properties(thread_pool PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

target_link_libraries(thread_pool PUBLIC pthread)