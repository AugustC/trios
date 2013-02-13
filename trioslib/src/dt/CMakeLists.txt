PROJECT(TRIOS_TREE)

set(TRIOS_TREE_SRCS
    CARTTrainer.cpp
    integration.cpp
)

INCLUDE_DIRECTORIES(include)

add_library(triostree ${TRIOS_TREE_SRCS})
target_link_libraries(triostree ${Boost_LIBRARIES} triosbasic  m)

