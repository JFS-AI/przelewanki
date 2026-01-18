#include <iostream>
#include <numeric>
#include <algorithm>
#include <array>
#include <span>
#include <cstdint>
#include <map>
#include <queue>
#include <climits>
#include <bit>
#include <vector>

constexpr int maxN = 11;
int n;

struct Stan {
    // 1. Rezerwujemy pamięć na max (na stosie, nie na stercie)
    std::array<int, maxN> buffer; 
    
    // Konstruktor domyślny
    Stan() {}

    // 2. Pomocniczy widok na aktywne dane
    std::span<const int> data() const {
        return {buffer.data(), static_cast<unsigned int>(n)};
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

std::map<Stan, int> mapa; // wrzucic do funkcji + dodac static

class kolejka012 {
	
public:
	std::vector<Stan> q[3];

	void przesunKolejki() {
		q[0].clear();
		std::swap(q[0], q[1]);
		std::swap(q[1], q[2]);
		q[2].clear();
	}
	bool isEmpty() const {
		return q[0].empty();
	}
	void push(const Stan& s, int krok) {
		q[krok].push_back(s);
	}
};

// bool czyStanWygrywajacy


kolejka012 kol;
void pushJesliNowy(const Stan& s, int nrRuchu, int silaRuchu) {
	nrRuchu += silaRuchu;
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
bool czyStanWygrywajacy(const Stan& s) {
	for(int i = 0; i < n; i++) {
		if(!czyPustoPelne[i] && s[i] != koniec[i])
			return false;
	}
	return true;
}
class AtomicSolver {
private:
    std::vector<int> _items; // Dodatnie dla Zielonych, ujemne dla Czerwonych

public:
    // Konstruktor przyjmuje:
    // green_offers: ile bomb mają poszczególni zieloni
    // red_demands: ile miejsca brakuje poszczególnym czerwonym
    AtomicSolver(const std::vector<int>& green_offers, const std::vector<int>& red_demands) {
        _items.reserve(green_offers.size() + red_demands.size());

        for (int val : green_offers) if (val > 0) _items.push_back(val);
        for (int val : red_demands)  if (val > 0) _items.push_back(-val);

        if (_items.size() > 15) {
            throw std::length_error("Algorytm obsluguje maksymalnie 15 aktywnych elementow.");
        }
    }

    [[nodiscard]] int solve() const {
        const int num_items = static_cast<int>(_items.size());
        if (num_items == 0) return 0;

        const int limit = 1 << num_items;
        std::vector<long long> sums(limit, 0);
        std::vector<int> dp(limit, 0);

        // 1. Prekalkulacja sum podzbiorów
        for (int mask = 1; mask < limit; ++mask) {
            int bit = std::countr_zero(static_cast<unsigned>(mask));
            sums[mask] = sums[mask ^ (1 << bit)] + _items[bit];
        }

        // 2. Programowanie dynamiczne (maksymalizacja liczby grup o sumie 0)
        for (int mask = 1; mask < limit; ++mask) {
            // Domyślnie dziedziczymy wynik z mniejszego podzbioru (usuwając jeden bit)
            for (int i = 0; i < num_items; ++i) {
                if (mask & (1 << i)) {
                    dp[mask] = std::max(dp[mask], dp[mask ^ (1 << i)]);
                }
            }

            // Jeśli suma maski wynosi 0, sprawdzamy podziały
            if (sums[mask] == 0) {
                int current_max = 1; // Maska sama w sobie jest poprawną grupą
                
                // Iteracja po podmaskach (O(3^N))
                for (int s = (mask - 1) & mask; s > 0; s = (s - 1) & mask) {
                    if (sums[s] == 0) {
                        current_max = std::max(current_max, dp[s] + dp[mask ^ s]);
                    }
                }
                dp[mask] = std::max(dp[mask], current_max);
            }
        }

        // Min. ruchów = liczba elementów - maksymalna liczba grup
        return num_items - dp[limit - 1];
    }
};
void operacjaPrzelania(const Stan& pocz, int i, int moc, int nrRuchu) {
	for(int j = 0; j < n; j++) {
		if(j == i) continue;
		Stan s = pocz;
		int przelew = std::min(s[i], pojemnosc[j] - s[j]);
		s[j] += przelew;
		s[i] -= przelew;
	
		pushJesliNowy(s, nrRuchu, moc);
	}
}
int solve() {
	int nrRuchu = 0;
	{
		Stan s;
		s.buffer.fill(0);
		pushJesliNowy(s, nrRuchu, 0);
	}
	
	while(!kol.isEmpty()) {
		int wynik = INT_MAX;
		bool czyProceduraKonca = false;
		
		for(Stan pocz : kol.q[0]) {
			/*
			if(czyStanWygrywajacy(pocz)) {
				std::vector<int> zieloni, czerwoni;
				for(int i = 0; i < n; i++) {
					if(pocz[i] == koniec[i])
						continue;
					
					if(koniec[i] == 0)
						zieloni.emplace_back(pocz[i]);

					else if(koniec[i] == pojemnosc[i])
						czerwoni.emplace_back(pojemnosc[i] - pocz[i]);
				}
				AtomicSolver as(zieloni, czerwoni);
				czyProceduraKonca = true;
				wynik = std::min(wynik, nrRuchu + as.solve());
			}
			*/
			if(pocz == koniec)
				return nrRuchu;

			for(int i = 0; i < n; i++) {
				if(pocz[i] < pojemnosc[i]) {
					Stan s = pocz;
					s[i] = pojemnosc[i];
					pushJesliNowy(s, nrRuchu, 1);
				}
				if(pocz[i] > 0) {
					Stan s = pocz;
					s[i] = 0;
					pushJesliNowy(s, nrRuchu, 1);
					operacjaPrzelania(pocz, i, 1, nrRuchu);
				}
			}
		}
		for(Stan pocz : kol.q[0]) {
			for(int i = 0; i < n; i++)
				if(pocz[i] < pojemnosc[i]) {
					Stan s = pocz;
					s[i] = pojemnosc[i];
					operacjaPrzelania(s, i, 2, nrRuchu);
				}
		}
		if(czyProceduraKonca) {
			return wynik;
		}
		kol.przesunKolejki();
		nrRuchu++;
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