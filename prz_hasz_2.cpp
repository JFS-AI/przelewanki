#include <iostream>
#include <numeric>
#include <algorithm>
#include <array>
#include <span>
#include <cstdint>
#include <unordered_map>
#include <deque>
#include <functional>

constexpr int maxN = 11;
constexpr std::size_t rozmiarPamieci = 2'000'000;
unsigned int n;

struct Stan {
    // 1. Rezerwujemy pamięć na max (na stosie, nie na stercie)
    std::array<int, maxN> buffer; 
    
    // Konstruktor domyślny
    Stan() {}

	explicit Stan(const std::vector<int>& vec) {
        std::ranges::copy(vec, buffer.begin());
    }

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


bool czyJestJedenPelnyLubPusty(const std::vector<int>& pojemnosc, const std::vector<int>& koniec) {
	for(int i = 0; i < n; i++) {
		if(koniec[i] == 0 || pojemnosc[i] == koniec[i])
			return true;
	}
	return false;
}
bool czyNwdJestOk(const std::vector<int>& pojemnosc, const std::vector<int>& koniec) {
	int nwdX = 0, nwdY = 0;
	for(int i = 0; i < n; i++) {
		nwdX = std::gcd(nwdX, pojemnosc[i]);
		nwdY = std::gcd(nwdY, koniec[i]);
	}
	return nwdY % nwdX == 0;
}
bool czyWarunkiKonieczneSpelnione(const std::vector<int>& x, const std::vector<int>& y) {
	return czyJestJedenPelnyLubPusty(x, y) && czyNwdJestOk(x, y);
}

std::unordered_map<Stan, int, StanHash> mapa; // wrzucic do funkcji + dodac static
std::vector<std::pair<Stan, int>> kol;
void pushJesliNowy(const Stan& s, int nrRuchu) {
	auto [it, inserted] = mapa.try_emplace(s, nrRuchu);
    if(inserted) {
        kol.emplace_back(s, nrRuchu);
    }
}
int solve(const std::vector<int>& x, const std::vector<int>& y) {
	Stan koniec(y), pojemnosc(x);
    mapa.reserve(rozmiarPamieci);
    kol.reserve(rozmiarPamieci);
    int head = 0;
	{
		Stan s;
		s.buffer.fill(0);
		pushJesliNowy(s, 0);
	}
	while(!kol.empty()) {
		Stan s = kol[head].first;
        int nrRuchu = kol[head].second;
        
		if(s == koniec)
			return nrRuchu;

        nrRuchu++;
		for(int i = 0; i < n; i++) {
            int temp = s[i];
			if(s[i] < pojemnosc[i]) {
				s[i] = pojemnosc[i];
				pushJesliNowy(s, nrRuchu);
                s[i] = temp;
			}
			if(s[i] > 0) {
				{
					s[i] = 0;
					pushJesliNowy(s, nrRuchu);
                    s[i] = temp;
				}
				for(int j = 0; j < n; j++) {
					if(j == i || s[j] == pojemnosc[j]) continue;

					int przelew = std::min(s[i], pojemnosc[j] - s[j]);
					s[j] += przelew;
					s[i] -= przelew;
					
					pushJesliNowy(s, nrRuchu);
                    s[j] -= przelew;
                    s[i] = temp;
				}
			}
		}
        
        //kol.pop_front();
        head++;
	}

	return -1;
}



int main() {
	std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
	std::cin >> n;

	int koniecPelny = 0, koniecPusty = 0;
	std::vector<int> pojemnosc, koniec;

	if(n == 0) {
		std::cout << 0 << "\n";
		return 0;
	}

	for(int i = 0; i < n; i++) {
		int x, y; // pojemnosc, stan docelowy;S
		std::cin >> x >> y;
		if(x == 0) { i--; n--; continue; } // kasujemy szklanki bez pojemności
		if(y == x) koniecPelny++;
		if(y == 0) koniecPusty++;
		
		pojemnosc.emplace_back(x);
		koniec.emplace_back(y);
	}

	if(!czyWarunkiKonieczneSpelnione(pojemnosc, koniec)) {
		std::cout << -1 << "\n";
		return 0;
	}

	if(koniecPelny + koniecPusty == n) {
		std::cout << koniecPelny << "\n";
		return 0;
	}

	if(n <= maxN) {
		std::cout << solve(pojemnosc, koniec) << "\n";
		return 0;
	}

	std::cerr << "reached end of main\n";
	return 1;
}