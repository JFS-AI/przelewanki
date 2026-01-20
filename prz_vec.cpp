#include <iostream>
#include <numeric>
#include <algorithm>
#include <array>
#include <optional>
#include <span>
#include <unordered_map>
#include <deque>
#include <functional>
#include <vector>


int n;


struct Stan {
    std::vector<int> buffer; 
    
    Stan() = default;

    explicit Stan(size_t size) : buffer(size, 0) {}

    explicit Stan(std::span<const int> data) : buffer(data.begin(), data.end()) {}

    bool operator==(const Stan& other) const {
        return buffer == other.buffer;
    }

    int& operator[](size_t index) { return buffer[index]; }
    const int& operator[](size_t index) const { return buffer[index]; }
};
struct StanHash {
    std::size_t operator()(const Stan& s) const {
        std::size_t seed = 0;
        for (int val : s.buffer) {
            // kombajn do haszy
            seed ^= std::hash<int>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};


template <typename Impl>
class BfsBaza {
protected:
    const Stan pojemnosc;
    std::vector<std::pair<Stan, int>> kol;
    int head = 0, tail = 0;
    int nrRuchu = -1;

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
public:
    using HashMap = std::unordered_map<Stan, int, StanHash>;
    HashMap& mapa;
    const HashMap& mapaOther;
    

    explicit BfsBaza(const Stan& s, const Stan& p, HashMap& moja, const HashMap& obca, std::size_t rP) 
                : pojemnosc(p), mapa(moja), mapaOther(obca) {
        
        kol.reserve(rP);

        pushJesliNowy(s); // potrzebujemy zacząć bfsa w jakimś punkcie
        nrRuchu++;
    }

    bool czyKolejkaPusta() const {
        return head > tail;
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

std::size_t policzLiczbeMozliwychStanow(const Stan& s) {
    std::size_t liczbaMozliwychStanow = 1;
    for(int val : s.buffer) {
        liczbaMozliwychStanow *= val + 1;
        if(liczbaMozliwychStanow > 10'000'000)
            break;
    }
    return liczbaMozliwychStanow;
}

int solve(const std::vector<int>& x, const std::vector<int>& y) {
	Stan pojemnosc(x), koniec(y), wyzerowany(n);

    std::size_t rozmiarPamieci = policzLiczbeMozliwychStanow(pojemnosc);
    std::unordered_map<Stan, int, StanHash> mapaDolu, mapaGory;
    mapaDolu.reserve(rozmiarPamieci);
    mapaGory.reserve(rozmiarPamieci);

    BfsDol dol(wyzerowany, pojemnosc, mapaDolu, mapaGory, rozmiarPamieci);
    BfsGora gora(koniec, pojemnosc, mapaGory, mapaDolu, rozmiarPamieci);

	while(!dol.czyKolejkaPusta() && !gora.czyKolejkaPusta()) {
        if(auto wynik = dol.wywolajCykl())  return *wynik;
        if(mapaGory.size() <= rozmiarPamieci)
            if(auto wynik = gora.wywolajCykl()) return *wynik;
    }

	return -1;
}


bool czyJestJedenPelnyLubPusty(const std::vector<int>& pojemnosc, const std::vector<int>& koniec) {
	for(int i = 0; i < n; i++) {
		if(koniec[i] == 0 || pojemnosc[i] == koniec[i])
			return true;
	}
	return false;
}
int policzNwd(const std::vector<int>& v) {
    int nwd = 0;
    for(int val : v) {
		nwd = std::gcd(nwd, val);
	}
    return nwd;
}
bool czyNwdJestOk(const std::vector<int>& pojemnosc, const std::vector<int>& koniec) {
	int nwdX = policzNwd(pojemnosc); 
    int nwdY = policzNwd(koniec);
	return nwdY % nwdX == 0;
}
bool czyWarunkiKonieczneSpelnione(const std::vector<int>& x, const std::vector<int>& y) {
	return czyJestJedenPelnyLubPusty(x, y) && czyNwdJestOk(x, y);
}
std::optional<int> czyPustoPelny(const std::vector<int>& pojemnosc, const std::vector<int>& koniec) {
    int koniecPelny = 0, koniecPusty = 0;
    for(int i = 0; i < n; i++) {
        if(koniec[i] == pojemnosc[i]) koniecPelny++;
		else if(koniec[i] == 0) koniecPusty++;
    }
    if(koniecPelny + koniecPusty == n) {
		return koniecPelny;
	}
    return std::nullopt;
}



int main() {
	std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
	std::cin >> n; // zmienna globalna

	std::vector<int> pojemnosc, koniec;

	for(int i = 0; i < n; i++) {
		int x, y; // pojemnosc, stan docelowy;
		std::cin >> x >> y;
		if(x == 0) { i--; n--; continue; } // kasujemy szklanki bez pojemności
		
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

	if(auto wynik = czyPustoPelny(pojemnosc, koniec)) {
        std::cout << *wynik << "\n";
        return 0;
    }

    // to ma szansę skasować dużo stanów
    int nwdX = policzNwd(pojemnosc);
    for(int i = 0; i < n; i++) {
        pojemnosc[i] /= nwdX;
        koniec[i] /= nwdX;
    }

	std::cout << solve(pojemnosc, koniec) << "\n";
	return 0;
}