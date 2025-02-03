# Projekt_CPP
Empire: Tower Defence </br>
Napisana w c++, korzystając z SDL-a.

> Uruchamianie programu:
Pobierz wszystkie pliki z github-a i je rozpakuj. Powinny znajdować się w tym samym folderze, wypakowane biblioteki muszą znajdować się w tym samym folderze).

Znajduje się tam plik ProjektGRA.exe

> Otwieranie projektu w Visual Studio i podłączanie bibliotek:

> Uruchamianie w Visual Studio 2022.
Kliknij "Otwórz projekt lub rozwiązanie".
Wskaż plik .sln w folderze Gry i kliknij "Otwórz".
Podłączenie bibliotek SDL2 (jeśli są w tym samym folderze co projekt)

Wejdź w Project > Properties.
Przejdź do C/C++ > General > Additional Include Directories (alt + F7) i dodaj:

SDL2-devel-2.30.8-VC\include

SDL2_ttf-2.24.0\include

Przejdź do Linker > General > Additional Library Directories i dodaj:

SDL2-devel-2.30.8-VC\lib\x64

SDL2_ttf-2.24.0\lib\x64

Przejdź do Linker > Input > Additional Dependencies i dopisz:

SDL2.lib

SDL2_ttf.lib
