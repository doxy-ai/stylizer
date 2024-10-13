# Waylib

Waylib is an experimental game engine based around stylization. To this end the engine is designed to provide some convenient APIs but also directly expose its raw underlying APIs for when convenience isn't enough, thus the internal APIs aren't just an implementation detail but for some workflows an essential part of the engine itself. To this end WebGPU (specifically Dawn) was chosen as the implementation API.

Waylib is heavily inspired by Raylib (the title was originally WebGPU Raylib) but has since expanded its scope to support various stylized workflows. Due to the extreme customizability certain stylization techniques need many of the raw internals are accessible, and the entire engine is delivered using a simple C like API. Many stylization techniques need access to intermediate buffers, thus the engine is optimized around deferred rendering where gbuffers are produced but care is taken to explicitly limit which buffers to create.

Additionally the engine is designed around being increadibly modular, this library includes several such modules all of which can be ignored if desired:
1) Core - WebGPU integrations and core rendering code, the core of the engine which every module assumes is present. Also provides some interface structures which other modules are expected to adhere to.
2) Window - Module which creates a window (using GFLW) and an associated surface to render to.
3) IMG - Provides simple generic image loading of common texture formats like png, jpg, and hdr (stb_image formats).
4) OBJ - Provides a simple obj model loader.

## License

All of the code in modules is MIT licensed, additional thirdparty dependencies may have different liceneses, here they are in summary but feel free to explore in more detail:


All of the images and models in the resources folder are creative commons (TODO: Version) licensed.
