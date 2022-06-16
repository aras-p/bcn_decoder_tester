﻿cmake_minimum_required (VERSION 3.8)

project ("bcn_decoder_tester")

add_executable (bcn_decoder_tester)
add_definitions(-DNOMINMAX)
target_sources(bcn_decoder_tester PRIVATE
	src/main.cpp
	src/dds_loader.cpp
)
target_compile_features(bcn_decoder_tester PRIVATE cxx_std_17)
target_include_directories(bcn_decoder_tester PRIVATE
)