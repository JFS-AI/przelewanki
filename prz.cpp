#include <iostream>
#include <numeric>

int n, pojemnosc[200], koniec[200];

bool czyJestJedenPelnyLubPusty() {
	bool wynik = false;
	for(int i = 0; i < n; i++) {
		if(koniec[i] == 0 || pojemnosc[i] = koniec[i])
			wynik = true;
	}
	return wynik;
}
bool czyNwdJestOk() {
	int nwdX = 0, nwdY = 0;
	for(int i = 0; i < n; i++) {
		nwdX = std::gcd(nwdX, pojemnosc[i]);
		nwdY = std::gcd(nwdY, koniec[i]);
	}
	return nwdY % nwdX == 0;
}
bool czyWarunkiKonieczneSpelnione() {
	return czyJestJedenPelnyLubPusty() && czyNwdJestOk();
}

int main() {
	std::cin >> n;

	for(int i = 0; i < n; i++) {
		std::cin >> pojemnosc[i] >> koniec[i];
		if(pojemnosc[i] == 0) { i--; n--; } // kasujemy szklanki bez pojemności
	}


	if(!czyWarunkiKonieczneSpelnione()) {
		std::cout << -1;
		return 0;
	}


}
