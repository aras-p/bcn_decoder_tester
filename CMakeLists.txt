cmake_minimum_required (VERSION 3.8)

project ("bcn_decoder_tester")

add_executable (bcn_decoder_tester)
add_definitions(-DNOMINMAX)
target_sources(bcn_decoder_tester PRIVATE
	src/main.cpp
	src/dds_loader.cpp
    src/dds_loader.h
    libs/bc7enc/bc7decomp.cpp
    libs/bc7enc/bc7decomp.h
    libs/bc7enc/rgbcx.h
    libs/DirectXTex/DirectXTex/BC.cpp
    libs/DirectXTex/DirectXTex/BC.h
    libs/DirectXTex/DirectXTex/BC4BC5.cpp
    libs/DirectXTex/DirectXTex/BC6HBC7.cpp
    libs/swiftshader/BC_Decoder.cpp
    libs/swiftshader/BC_Decoder.hpp
    libs/swiftshader/Debug.cpp
)
target_compile_features(bcn_decoder_tester PRIVATE cxx_std_17)
target_include_directories(bcn_decoder_tester PRIVATE
    libs/DirectX-Headers/include
    libs/DirectX-Headers/include/wsl/stubs
    libs/DirectXMath/Inc
    libs
)
