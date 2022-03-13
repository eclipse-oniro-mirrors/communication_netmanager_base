include_directories(network_module/include)

add_library(network SHARED
    network_module/src/network_module.cpp
    )