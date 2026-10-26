## ZALEŻNOŚCI
- [Git](https://git-scm.com/)
- [CMake](https://cmake.org/)
- [MinGW (MSYS2)](https://www.msys2.org/)

## INSTALACJA MSYS2
#### Pobierz [instalator](https://www.msys2.org/) z oficjalnego źródła
![Pobierz MSYS2](../Images/pobierz_msys2.png)
***[!]** pamiętaj o pobraniu wersji na architekture **AMD64 (x86_64)** zamiast na ARM64*
*instrukcje odnośnie instalacji znajdziesz na oficjalnym źródle*
#### Zainstaluj pakiet MinGW w środowisku MSYS2
***[!]** pamiętaj by urochomić środowisko jako administrator*
```bash
$ pacman -S mingw-w64-ucrt-x86_64-gcc
```
![Instalacja MinGW](../Images/pacman_-S_mingw.png)
#### Dodaj lokalizacje plików binarnych MinGW do zmiennej środowiskowej PATH
![Dodaj MinGW do PATH](../Images/dodaj_mingw_do_path.png)
#### Teraz MinGW powinien działać z poziomu Windows Terminal
![MinGW w Terminalu](../Images/mingw_terminal.png)


## KONFIGURACJA ŚRODOWISKA
#### W terminalu [**MSYS2 UCRT64**](https://www.msys2.org/docs/environments/) zsynchronizuj i zaaktualizuj środowisko
``` bash
$ pacman -Syu
```
![Updating Pacman](../Images/pacman_-Syu.png)
#### Następnie zainstaluj zależności dla środowiska MinGW
``` bash
$ pacman -S --needed \
   mingw-w64-ucrt-x86_64-toolchain \
   mingw-w64-ucrt-x86_64-cmake \
   mingw-w64-ucrt-x86_64-ninja \
   mingw-w64-ucrt-x86_64-glfw
```
![image alt](../Images/pacman_-S_--needed.png)
*podczas instalacji możesz dostrzec ostszerzenia o brakujących opcjonalnych zależnościach i niepoprawnie wykonywanych komendach, zignoruj je*



## KONFIGURACJA I BUDOWA PROJEKTU
#### Sklonuj repozytorium projektu z GitHuba
``` bash
$ git clone https://github.com/MaticLibrary/CraftVortex.git
```
![Sklonuj projekt](../Images/git_clone.png)
#### Wejdź do katalogu głównego projektu i skonfiguruj pliki Make
```bash
$ cmake -S . -B build -G "MinGW Makefiles" `
   -DCMAKE_BUILD_TYPE=Release `
   -DCMAKE_C_COMPILER=gcc `
   -DCMAKE_CXX_COMPILER=g++
```
![Konfiguracja CMake](../Images/cmake_-S_-B_build_-G_MinGW.png)
#### Zbuduj projekt
``` bash
$ cmake --build build --parallel
```
![Zbuduj projekt](../Images/cmake_--build_build_--parallel.png)

## URUCHOM PROJEKT
![Uruchom projekt](../Images/run.png)