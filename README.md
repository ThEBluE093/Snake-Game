# SDL2 Snake Game (C++)

Klasyczna gra Snake napisana w C++ z wykorzystaniem biblioteki SDL2. Projekt zawiera mechaniki dynamicznego wzrostu prędkości, system bonusów oraz lokalny ranking najlepszych wyników.

### 🎮 Cechy gry
Dynamiczna trudność: Prędkość węża wzrasta co 10 sekund o 20%.
System bonusów: 🔵 Niebieskie kropki: Standardowe jedzenie (zwiększa długość i punkty).
 🔴 Czerwone kropki (Power-upy): Pojawiają się na ograniczony czas. Mogą skrócić węża lub spowolnić rozgrywkę.
System Rankingowy: Zapisywanie 3 najlepszych wyników do pliku `wyniki.txt` wraz z pseudonimem gracza.
Autorskie renderowanie: Gra nie używa gotowych tekstur dla obiektów, lecz rysuje je bezpośrednio na powierzchni (SDL_Surface) za pomocą własnych funkcji graficznych.

### 🛠️ Technologie i kompilacja
Język: C++
Biblioteka: SDL2 (wymagana ścieżka `./SDL2-2.0.10/`)
Assety: Plik `cs8x8.bmp` (charset do renderowania napisów).

Aby skompilować projekt, upewnij się, że biblioteki SDL2 są poprawnie podlinkowane w Twoim środowisku.
Kroki kompilacji i uruchomienia:
1.) /comp
2.) /snake
