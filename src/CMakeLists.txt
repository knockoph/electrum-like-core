add_executable(elc-cc coreclient/main.cpp)
target_link_libraries(elc-cc univalue)
add_subdirectory(univalue)
