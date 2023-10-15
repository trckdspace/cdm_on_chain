project(trckdspacechain)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall")



include_directories( 
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/extern/perturb/include
    /usr/include/eigen3
    ${Pangolin_INCLUDE_DIRS}
)

link_directories(
    ${PROJECT_SOURCE_DIR}/extern/perturb/lib
)

message(${PROJECT_SOURCE_DIR}/extern/perturb/lib)

add_executable(run_chain src/Main.cpp src/Block.cpp 
    src/BlockChain.cpp  
    src/SHA256.cpp 
    src/Transaction.cpp)
target_link_libraries(run_chain perturb ssl crypto)

