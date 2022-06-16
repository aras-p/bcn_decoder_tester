# Testing various BCn decoding libraries

### Libraries:

* `bc7enc`: https://github.com/richgel999/bc7enc, 2021 Feb 8 (1a4e9db). MIT or public domain.
* `bcdec`: https://github.com/iOrange/bcdec, 2022 May 10 (ee317d1). Unlicense.
* `DirectXTex`: https://github.com/microsoft/DirectXTex, 2022 Jun 6 (679538e). MIT.
  Does not easily build on non-Windows: requires bits of DirectX-Headers and DirectXMath
  github projects, as well as an empty `<malloc.h>` and some stub `<sal.h>`.
* `icbc`: https://github.com/castano/icbc, 2022 Jun 3 (502ec5a). MIT.
