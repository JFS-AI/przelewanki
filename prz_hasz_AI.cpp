#include <iostream>
#include <numeric>
#include <algorithm>
#include <array>
#include <span>
#include <cstdint>
#include <unordered_map>
#include <queue>
#include <functional>

constexpr int TABLE_SIZE = 1 << 21;
constexpr int maxN = 11;
unsigned int n;

struct Stan {
    // 1. Rezerwujemy pamięć na max (na stosie, nie na stercie)
    std::array<int, maxN> buffer; 

    // Konstruktor domyślny
    Stan() {}

    // Hashowanie wbudowane dla szybkości
    // Używamy FNV-1a hash lub zmodyfikowanego boosta
    uint64_t hash() const {
        uint64_t seed = 0xcbf29ce484222325; // FNV offset basis
        for(int i = 0; i < n; i++) {
            seed ^= buffer[i];
            seed *= 0x100000001b3; // FNV prime
        }
        return seed;
    }

    // 2. Pomocniczy widok na aktywne dane
    std::span<const int> data() const {
        return {buffer.data(), n};
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
struct FastSet {
    struct Entry {
        Stan key;
        int dist;
        bool used;
    };
    
    // Alokujemy raz dużą tablicę. 
    // unique_ptr pozwala trzymać to na stercie (stos by wybuchł), ale bez narzutu mapy.
    // Używamy vectora, żeby RAII posprzątało, ale access jest raw.
    std::vector<Entry> table;

    FastSet() {
        table.resize(TABLE_SIZE, {{}, -1, false});
    }

    // Zwraca wskaźnik do dystansu, jeśli znaleziono lub wstawiono nowy.
    // Jeśli wstawiono nowy, zwraca -1 w wartości pod wskaźnikiem (przed nadpisaniem).
    // Return: pair<int* dystans, bool czyNowy>
    std::pair<int*, bool> getDistPtr(const Stan& s) {
        uint64_t h = s.hash();
        size_t idx = h & (TABLE_SIZE - 1); // Szybkie modulo (bo size to potęga 2)

        while (true) {
            if (!table[idx].used) {
                // Znaleziono puste miejsce -> wstawiamy
                table[idx].key = s;
                table[idx].used = true;
                table[idx].dist = -1; // Oznaczenie "nowy"
                return {&table[idx].dist, true};
            }
            if (table[idx].key == s) {
                // Znaleziono ten sam stan
                return {&table[idx].dist, false};
            }
            // Kolizja - idziemy dalej (Linear Probing)
            idx = (idx + 1) & (TABLE_SIZE - 1);
        }
    }
};


Stan pojemnosc, koniec, czyPustoPelne;

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
		std::swap(q[0], q[1]);
	}

public:
	bool isEmpty(int& krok) {
		if(q[0].empty()) {
			krok++;
			tryUpdating();
		}
		return q[0].empty();
	}
	void push(const Stan& s, int krok) {
		q[krok].push(s);
	}
	Stan pop() {
		Stan wynik = q[0].front();
		q[0].pop();
		return wynik;
	}
};

// bool czyStanWygrywajacy

FastSet mapa;
kolejka012 kol;
inline void pushJesliNowy(const Stan& s, int dist, int silaRuchu) {
	auto result = mapa.getDistPtr(s);
	if (result.second) { // Jeśli to nowy stan
        *(result.first) = dist; // Zapisz dystans
        kol.push(s, silaRuchu);
    }

}
int solve() {
	int nrRuchu = 0;
	{
		Stan s;
		s.buffer.fill(0);
		pushJesliNowy(s, nrRuchu, 0);
	}
	while(!kol.isEmpty(nrRuchu)) {
		Stan pocz = kol.pop();
		if(pocz == koniec)
			return nrRuchu;

		for(int i = 0; i < n; i++) {
			if(pocz[i] < pojemnosc[i]) {
				Stan s = pocz;
				s[i] = pojemnosc[i];
				pushJesliNowy(s, nrRuchu, 1);
			}
			if(pocz[i] > 0) {
				{
					Stan s = pocz;
					s[i] = 0;
					pushJesliNowy(s, nrRuchu, 1);
				}
				for(int j = 0; j < n; j++) {
					if(j == i) continue;
					Stan s = pocz;
					int przelew = std::min(s[i], pojemnosc[j] - s[j]);
					s[j] += przelew;
					s[i] -= przelew;
					
					pushJesliNowy(s, nrRuchu, 1);
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
		if(pojemnosc[i] == 0) 
			{ i--; n--; } // kasujemy szklanki bez pojemności
		else 
			czyPustoPelne[i] = (koniec[i] == 0 || pojemnosc[i] == koniec[i]);
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