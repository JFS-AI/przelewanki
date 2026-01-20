#include <iostream>
#include <numeric>
#include <algorithm>
#include <array>
#include <span>
#include <cstdint>
#include <unordered_map>
#include <deque>
#include <functional>
#include <vector>

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

template <typename Impl>
class BfsBaza {
protected:
    const Stan pojemnosc;
    std::vector<std::pair<Stan, int>> kol;
    int head = 0, tail = 0;
    int nrRuchu = -1;

public:
    using HashMap = std::unordered_map<Stan, int, StanHash>;
    HashMap& mapa;
    const HashMap& mapaOther;
    

    explicit BfsBaza(const Stan& s, const Stan& p, HashMap& moja, const HashMap& obca) 
                : pojemnosc(p), mapa(moja), mapaOther(obca) {
        
        kol.reserve(rozmiarPamieci);

        pushJesliNowy(s);
        nrRuchu++;
    }

    bool czyKolejkaPusta() const {
        return head > tail;
    }

    void pushJesliNowy(const Stan& s) {
        auto [it, inserted] = mapa.try_emplace(s, nrRuchu + 1);
        if(inserted) {
            kol.emplace_back(s, nrRuchu + 1);
            tail++;
        }
    }
    std::optional<int> czyNaDrugiejMapie(const Stan& s) const {
        auto search = mapaOther.find(s);
        if(search != mapaOther.end())
            return nrRuchu + 1 + search->second;

        return std::nullopt;
    }

    std::optional<int> wywolajCykl() {
        while(kol[head].second == nrRuchu) {
            Stan s = kol[head].first;

            for(int i = 0; i < n; i++) {
                
                if(auto wynik = static_cast<Impl*>(this)->funkcjaNapelnienia(i, s))
                    return wynik;

                if(auto wynik = static_cast<Impl*>(this)->funckjaOproznienia(i, s))
                    return wynik;
                
                for(int j = 0; j < n; j++) {
                    if(j == i) continue;
                        
                    if(auto wynik = static_cast<Impl*>(this)->funkcjaPrzelewu(i, j, s))
                        return wynik;
                }
            }
            head++;
        }

        nrRuchu++;
        return std::nullopt;
    }
};

class BfsDol : public BfsBaza<BfsDol> {
public:
    using BfsBaza<BfsDol>::BfsBaza;
    std::optional<int> funkcjaNapelnienia(int i, Stan& s) {
        if(s[i] == pojemnosc[i]) 
            return std::nullopt;

        int temp = s[i];
        s[i] = pojemnosc[i];
        if(auto wynik = czyNaDrugiejMapie(s)) return wynik;
        pushJesliNowy(s);
        s[i] = temp;

        return std::nullopt;
    }
    std::optional<int> funckjaOproznienia(int i, Stan& s) {
        if(s[i] == 0)
            return std::nullopt;
        
        int temp = s[i];
        s[i] = 0;
        if(auto wynik = czyNaDrugiejMapie(s)) return wynik;
        pushJesliNowy(s);
        s[i] = temp;

        return std::nullopt;
    }
    std::optional<int> funkcjaPrzelewu(int from, int to, Stan& s) {
        

        int przelew = std::min(s[from], pojemnosc[to] - s[to]);
        s[to] += przelew;
        s[from] -= przelew;
                        
        if(auto wynik = czyNaDrugiejMapie(s)) return wynik;
        pushJesliNowy(s);
        s[to] -= przelew;
        s[from] += przelew;

        return std::nullopt;
    }
};
class BfsGora : public BfsBaza<BfsGora> {
public:
    using BfsBaza<BfsGora>::BfsBaza;
    std::optional<int> funkcjaNapelnienia(int i, Stan& s) {
        if(s[i] < pojemnosc[i]) 
            return std::nullopt;

        for(int poziomWody = 0; poziomWody < pojemnosc[i]; poziomWody++) {
            s[i] = poziomWody;
            if(auto wynik = czyNaDrugiejMapie(s)) return wynik;
            pushJesliNowy(s);
        }
        s[i] = pojemnosc[i];

        return std::nullopt;
    }
    std::optional<int> funckjaOproznienia(int i, Stan& s) {
        if(s[i] > 0)
            return std::nullopt;
            
        for(int poziomWody = 1; poziomWody <= pojemnosc[i]; poziomWody++) {
            s[i] = poziomWody;
            if(auto wynik = czyNaDrugiejMapie(s)) return wynik;
            pushJesliNowy(s);
        }
        s[i] = 0;

        return std::nullopt;
    }
    std::optional<int> funkcjaPrzelewu(int from, int to, Stan& s) {
        if(s[from] > 0 && s[to] < pojemnosc[to]) return std::nullopt;
        
        for(int przelew = 1; przelew <= std::min(s[to], pojemnosc[from] - s[from]); przelew++) {
            s[to] -= przelew;
            s[from] += przelew;
                            
            if(auto wynik = czyNaDrugiejMapie(s)) return wynik;
            pushJesliNowy(s);
            s[to] += przelew;
            s[from] -= przelew;
        }
        return std::nullopt;
    }
};

int solve(const std::vector<int>& x, const std::vector<int>& y) {
	Stan pojemnosc(x), koniec(y), wyzerowany;
	wyzerowany.buffer.fill(0);

    std::unordered_map<Stan, int, StanHash> mapaDolu, mapaGory;
    mapaDolu.reserve(rozmiarPamieci);
    mapaGory.reserve(rozmiarPamieci);

    BfsDol dol(wyzerowany, pojemnosc, mapaDolu, mapaGory);
    BfsGora gora(koniec, pojemnosc, mapaGory, mapaDolu);

	while(!dol.czyKolejkaPusta() && !gora.czyKolejkaPusta()) {
        if(auto wynik = dol.wywolajCykl())  return *wynik;
        if(auto wynik = gora.wywolajCykl()) return *wynik;
    }

	return -1;
}



int main() {
	std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
	std::cin >> n;

	int koniecPelny = 0, koniecPusty = 0;
	std::vector<int> pojemnosc, koniec;

	for(int i = 0; i < n; i++) {
		int x, y; // pojemnosc, stan docelowy;
		std::cin >> x >> y;
		if(x == 0) { i--; n--; continue; } // kasujemy szklanki bez pojemności
		if(y == x) koniecPelny++;
		if(y == 0) koniecPusty++;
		
		pojemnosc.emplace_back(x);
		koniec.emplace_back(y);
	}

	if(n == 0) {
		std::cout << 0 << "\n";
		return 0;
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