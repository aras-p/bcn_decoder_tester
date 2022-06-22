cmake_minimum_required (VERSION 3.8)

project ("bcn_decoder_tester")

add_executable (bcn_decoder_tester)
add_definitions(-DNOMINMAX)
target_sources(bcn_decoder_tester PRIVATE
	src/main.cpp
	src/dds_loader.cpp
    src/dds_loader.h
    libs/bc7enc_rdo/bc7decomp.cpp
    libs/bc7enc_rdo/bc7decomp.h
    libs/bc7enc_rdo/rgbcx.cpp
    libs/bc7enc_rdo/rgbcx.h
    libs/DirectXTex/DirectXTex/BC.cpp
    libs/DirectXTex/DirectXTex/BC.h
    libs/DirectXTex/DirectXTex/BC4BC5.cpp
    libs/DirectXTex/DirectXTex/BC6HBC7.cpp
    libs/swiftshader/BC_Decoder.cpp
    libs/swiftshader/BC_Decoder.hpp
    libs/swiftshader/Debug.cpp
    libs/etcpak/BlockData.cpp
    libs/etcpak/BlockData.hpp
    libs/libsquish/alpha.cpp
    libs/libsquish/clusterfit.cpp
    libs/libsquish/colourblock.cpp
    libs/libsquish/colourfit.cpp
    libs/libsquish/colourset.cpp
    libs/libsquish/maths.cpp
    libs/libsquish/rangefit.cpp
    libs/libsquish/singlecolourfit.cpp
    libs/libsquish/squish.cpp
    libs/libsquish/squish.h
    libs/xxHash/xxhash.c
    libs/xxHash/xxhash.h
    libs/ConvectionKernels/ConvectionKernels_BC67.cpp
    libs/ConvectionKernels/ConvectionKernels_BC67.h
    libs/ConvectionKernels/ConvectionKernels_BC6H_IO.cpp
    libs/ConvectionKernels/ConvectionKernels_BC7_PrioData.cpp
    libs/ConvectionKernels/ConvectionKernels_IndexSelector.cpp
    libs/ConvectionKernels/ConvectionKernels_Util.cpp
)
target_compile_features(bcn_decoder_tester PRIVATE cxx_std_17)

if (NOT WIN32) 
    target_include_directories(bcn_decoder_tester PRIVATE
        libs/DirectX-Headers/include
        libs/DirectX-Headers/include/wsl/stubs
        libs/DirectXMath/Inc
        libs
    )
endif()
