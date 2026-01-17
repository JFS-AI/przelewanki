#include <iostream>

int n, pojemnosc[200], koniec[200];

bool czyJestJedenPelnyLubPusty() {
	bool wynik = false;
	for(int i = 0; i < n; i++) {
		if(koniec[i] == 0 || pojemnosc[i] = koniec[i])
			wynik = true;
	}
	return wynik;
}

bool czyWarunkiKonieczneSpelnione() {
	return czyJestJedenPelnyLubPusty();
}

int main() {
	std::cin >> n;

	for(int i = 0; i < n; i++) 
		std::cin >> pojemnosc[i] >> koniec[i];

	if(!czyWarunkiKonieczneSpelnione()) {
		std::cout << -1;
		return 0;
	}

	
}
