# Testing various BCn decoding libraries

### Libraries Tested:

Used as git submodules:

* `amd_cmp`: https://github.com/GPUOpen-Tools/compressonator, 2022 Jun 20 (b2b4ee3). MIT.
* `bc7enc_rdo`: https://github.com/richgel999/bc7enc_rdo, 2021 Dec 27 (e6990bc). MIT or public domain.
* `bcdec`: https://github.com/iOrange/bcdec, 2022 Jun 23 (e3ca028). Unlicense.
* `convection`: https://github.com/elasota/ConvectionKernels, 2022 Jun 23 (350416d). MIT.
* `DirectXTex`: https://github.com/microsoft/DirectXTex, 2022 Jun 6 (679538e). MIT.
  Does not easily build on non-Windows: requires bits of DirectX-Headers and DirectXMath
  github projects, as well as an empty `<malloc.h>` and some stub `<sal.h>`.
* `icbc`: https://github.com/castano/icbc, 2022 Jun 3 (502ec5a). MIT.

Embedded directly into the repo:

* `etcpak`: https://github.com/wolfpld/etcpak, 2022 Jun 4 (a77d5a3). BSD 3-clause.
  Just the needed source files, modified to leave only decompression bits.
* `mesa`: https://github.com/mesa3d/mesa, 2022 Jun 22 (ad3d6d9). MIT.
  Just the needed source files, modified to leave only decompression bits.
* `squish`: https://sourceforge.net/projects/libsquish/, 2019 Apr 25 (r110, v1.15). MIT.
  Upstream is not a Git repository.
* `swiftshader`: https://github.com/google/swiftshader, 2022 Jun 16 (2b79b2f). Apache-2.0.
  Minimal set of source files.

## Test Images:

I produced most of the .dds files under `test_images` folder myself, using a simple
"[export DDS](https://gist.github.com/aras-p/0f0b02aa193346be18e5f130f2704782)" script for Unity.
Within Unity, used various import options to produce BCn formats out of original images.

The original images are from:

* `ambientcg/`: various texture & HDRI files from [ambientCG](https://ambientcg.com/).
* `frymire`: University of Waterloo [image set](https://links.uwaterloo.ca/Repository.html).
* `kodim_23`: Kodak lossless [image suite](http://r0k.us/graphics/kodak/kodim23.html).
* `lythwood_room`: Lythwood Room HDRI from [Poly Haven](https://polyhaven.com/a/lythwood_room).
* `testcard`: BC2 DDS file from [bcdec](https://github.com/iOrange/bcdec/tree/main/test_images).
* `webp_gallery_3`: WebP Lossless and Alpha Gallery [page](https://developers.google.com/speed/webp/gallery2).
