#include <iostream>
#include <numeric>
#include <algorithm>
#include <array>
#include <span>
#include <cstdint>
#include <map>
#include <queue>


constexpr int maxN = 10;
int n;

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
    Stan() : size(n) {}

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

	int& operator[](size_t index) {
        return buffer[index];
    }

    // Wersja tylko do odczytu (const)
    const int& operator[](size_t index) const {
        return buffer[index];
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

std::map<Stan, int> mapa; // wrzucic do funkcji + dodac static
kolejka012 kol;
void pushJesliNowy(const Stan& s, int nrRuchu, int silaRuchu) {
	auto search = mapa.find(s);
	if(search == mapa.end()) {
		kol.push(s, silaRuchu);
		mapa.emplace(s, nrRuchu);
	}
	else if(search->second > nrRuchu) {
		kol.push(s, silaRuchu);
		search->second = nrRuchu;
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
					s[j] += s[i];
					int overflow = s[j] - pojemnosc[j];
					if(overflow > 0) {
						s[i] = overflow;
						s[j] = pojemnosc[j];
					}
					else
						s[i] = 0;
					
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
	pojemnosc.size = n; 
	koniec.size = n;
	czyPustoPelne.size = n;

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