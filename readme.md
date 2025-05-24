# vulkanbox

Hobby project created as a sandbox for learning Vulkan rendering API and other low level systems gradually.

Currently supports Windows only. Linux support planned.

## Building

Needs Vulkan SDK already installed on your machine.

Uses [mingen](https://github.com/BluTree/mingen) for compilation. Generates the project with it, and compile it.

## Dependencies

- [Dear ImGui](https://github.com/ocornut/imgui): Used for debug UI purposes.
- [mincore](https://github.com/BluTree/mincore): Core library for containers, type traits,...
- [WindowsHModular](https://github.com/Leandros/WindowsHModular): Modular headers for Win32 API.
- [stb](https://github.com/nothings/stb): Used for image loading header.
- [Superluminal](https://superluminal.eu/): (Optional) Profiling tool. Used to instrument the frame timing for a later use.
- [Vulkan](https://www.vulkan.org/): Cross-platform rendering API
- [Slang](https://shader-slang.org/): Shader language
- [yyjson](https://github.com/ibireme/yyjson): C Reader/Writer for JSON files
