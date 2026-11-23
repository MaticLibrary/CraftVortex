## WYMAGANE ZALEŻNOŚCI
| Narzędzie | Zastosowanie |
| --- | --- |
| [Git](https://git-scm.com/) | System kontroli wersji |
| [CMake](https://cmake.org/) | Konfiguracja projektu |
| [MinGW MSYS2 (GCC, G++)](https://www.mingw-w64.org/) | Kompilatory OpenSource |
| [Microsoft Visual C++ (MSVCP)](https://learn.microsoft.com/pl-pl/cpp/windows/latest-supported-vc-redist?view=msvc-170) | Kompilatory Microsoft |
| [GLFW (OpenGL Framework)](https://www.glfw.org/) | Obsługa kontekstu |
| [GLAD (OpenGL Loader Generator)](https://glad.dav1d.de/) | Wczytywanie funkcji OpenGL |
| [GLM (OpenGL Mathematics)](https://github.com/g-truc/glm) | Biblioteka matematyczna |
| [Fast Noise Lite](https://github.com/Auburn/FastNoiseLite) | Generacja szumu |

## BUDOWA PROJEKTU
- [Kompilacja na Windowsie przez MinGW64](https://github.com/MaticLibrary/CraftVortex/blob/main/docs/Windows_instalacja_przez_MinGW64.md)
- [Kompilacja na Windowsie przez MSVCP]()

## Budowanie przez CMake
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```
```bash
cmake --build build --config Release -j
```
