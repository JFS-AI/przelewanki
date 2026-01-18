#include <iostream>
#include <numeric>
#include <algorithm>
#include <array>
#include <span>
#include <cstdint>
#include <unordered_map>
#include <queue>
#include <functional>

constexpr int maxN = 11;
unsigned int n;

struct Stan {
    // 1. Rezerwujemy pamięć na max (na stosie, nie na stercie)
    std::array<int, maxN> buffer; 
    
    // Konstruktor domyślny
    Stan() {}

    // 2. Pomocniczy widok na aktywne dane
    std::span<const int> data() const {
        return {buffer.data(), n};
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

	int& operator[](size_t index) {
        return buffer[index];
    }

    // Wersja tylko do odczytu (const)
    const int& operator[](size_t index) const {
        return buffer[index];
    }
};
struct StanHash {
    std::size_t operator()(const Stan& s) const {
        std::size_t seed = 0;
        // Iterujemy tylko po aktywnych elementach (dzięki span)
        for (int val : s.data()) {
            // Klasyczny algorytm "hash combine" (używany np. w Boost)
            // Mieszamy bity, aby (1,0) miało inny hash niż (0,1)
            seed ^= std::hash<int>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};


Stan pojemnosc, koniec;

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

std::unordered_map<Stan, int, StanHash> mapa; // wrzucic do funkcji + dodac static
std::queue<std::pair<Stan, int>> kol;
void pushJesliNowy(const Stan& s, int nrRuchu) {
	auto search = mapa.find(s);
	if(search == mapa.end()) {
		kol.emplace(s, nrRuchu);
		mapa.emplace(s, nrRuchu);
    }
}
int solve() {
	{
		Stan s;
		s.buffer.fill(0);
		pushJesliNowy(s, 0);
	}
	while(!kol.empty()) {
		Stan pocz = kol.front().first;
        int nrRuchu = kol.front().second;
        kol.pop();
		if(pocz == koniec)
			return nrRuchu;

        nrRuchu++;
		for(int i = 0; i < n; i++) {
			if(pocz[i] < pojemnosc[i]) {
				Stan s = pocz;
				s[i] = pojemnosc[i];
				pushJesliNowy(s, nrRuchu);
			}
			if(pocz[i] > 0) {
				{
					Stan s = pocz;
					s[i] = 0;
					pushJesliNowy(s, nrRuchu);
				}
				for(int j = 0; j < n; j++) {
					if(j == i || pocz[j] == pojemnosc[j]) continue;
                    
					Stan s = pocz;
					int przelew = std::min(s[i], pojemnosc[j] - s[j]);
					s[j] += przelew;
					s[i] -= przelew;
					
					pushJesliNowy(s, nrRuchu);
				}
			}
		}
	}

	return -1;
}

int main() {
	std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
	std::cin >> n;

	for(int i = 0; i < n; i++) {
		std::cin >> pojemnosc[i] >> koniec[i];
		if(pojemnosc[i] == 0) { i--; n--; } // kasujemy szklanki bez pojemności
	}

	if(n == 0) {
		std::cout << 0 << "\n";
		return 0;
	}

	if(!czyWarunkiKonieczneSpelnione()) {
		std::cout << -1 << "\n";
		return 0;
	}

	std::cout << solve() << "\n";
	return 0;
}