PROJECT(TRIOSLIB)
cmake_minimum_required(VERSION 2.8)

#find_package(OpenCV)
#find_package(OpenMP)

#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TRIOS_REQUIRED_C_FLAGS} ${OpenMP_C_FLAGS} -fopenmp")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TRIOS_REQUIRED_CXX_FLAGS}")
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${TRIOS_REQUIRED_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS} -fopenmp")

subdirs(unit_tests)

add_library(trios
# ---- Dataset definition ----
dataset.c
dataset_io.c

# ---- Image structure ----
img.c
img_io.c

# ---- Feature Extraction  ----
#raw_pattern_data.c -- funções para padrões (compare, alloc, etc)
#sample_tree.c -- xplGG
#feature_extraction.c -- collec

# ---- Miscelaneous and utility functions ----
io_header.c
error.c


)