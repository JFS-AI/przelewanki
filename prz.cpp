#include <iostream>
#include <numeric>
#include <algorithm>
#include <array>
#include <span>
#include <cstdint>
#include <map>

constexpr maxN = 20;

int n, pojemnosc[maxN], koniec[maxN];
bool czyPustoPelne[maxN];

struct Stan {
    // 1. Rezerwujemy pamięć na max (na stosie, nie na stercie)
    std::array<int, maxN> buffer; 
    uint8_t size; // Faktyczna liczba elementów (n)

    // Konstruktor pomocniczy (opcjonalny, dla wygody)
    Stan(std::initializer_list<int> list) {
        size = static_cast<uint8_t>(list.size());
        std::copy(list.begin(), list.end(), buffer.begin());
    }
    
    // Konstruktor domyślny
    Stan() : size(0) {}

    // 2. Pomocniczy widok na aktywne dane
    std::span<const int> data() const {
        return {buffer.data(), size};
    }

    // 3. Operator <=> (C++20/23) - klucz do działania w mapie
    // Porównujemy tylko 'size' elementów, ignorujemy śmieci na końcu tablicy
    auto operator<=>(const Stan& other) const {
        return std::lexicographical_compare_three_way(
            data().begin(), data().end(),
            other.data().begin(), other.data().end()
        );
    }

    // Wymagane, aby operator <=> działał poprawnie jako klucz
    bool operator==(const Stan& other) const {
        return std::equal(data().begin(), data().end(), 
                          other.data().begin(), other.data().end());
    }
};



bool czyJestJedenPelnyLubPusty() {
	for(int i = 0; i < n; i++) {
		if(koniec[i] == 0 || pojemnosc[i] == koniec[i])
			return true;
	}
	return false;
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

class kolejka012 {
	std::queue<Stan> q[3];
	
	void tryUpdating() {

	}

public:
	bool isEmpty() {
		if(q0.empty())
			tryUpdating();
		return q0.empty();
	}
	void push(Stan s, int krok) {
		q[krok].push(s);
	}
	Stan pop() {
		Stan wynik = q[0].front();
		q[0].pop();
		return wynik;
	}
}

bool czyStanWygrywajacy

std::map<Stan, int> mapa; // wrzucic do funkcji + dodac static
kolejka012 kol;
int solve() {
	{
		Stan s();
		for(int& x : s.buffer)
			x = 0;
		kol.push(s, 0);
	}
	int i = 0;
	while(!kol.isEmpty()) {
		Stan pocz = kol.pop();
		
	}
}

int main() {
	std::cin >> n;

	for(int i = 0; i < n; i++) {
		std::cin >> pojemnosc[i] >> koniec[i];
		if(pojemnosc[i] == 0) 
			{ i--; n--; } // kasujemy szklanki bez pojemności
		else 
			czyPustoPelne[i] = (koniec[i] == 0 || pojemnosc[i] == koniec[i]);
	}

	if(n == 0) {
		std::cout << 0;
		return 0;
	}

	if(!czyWarunkiKonieczneSpelnione()) {
		std::cout << -1;
		return 0;
	}

	
}