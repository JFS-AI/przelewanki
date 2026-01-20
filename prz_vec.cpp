#include <iostream>
#include <numeric>
#include <algorithm>
#include <array>
#include <optional>
#include <span>
#include <cstdint>
#include <unordered_map>
#include <deque>
#include <functional>
#include <vector>

std::size_t rozmiarPamieci = -1;
int n;

struct Stan {
    std::vector<int> buffer; 
    
    Stan() = default;

    explicit Stan(size_t size) : buffer(size, 0) {}

    explicit Stan(std::span<const int> data) : buffer(data.begin(), data.end()) {}

    std::span<const int> data() const {
        return buffer; // automatyczna konwersja vector -> span
    }

    bool operator==(const Stan& other) const {
        return buffer == other.buffer;
    }

    int& operator[](size_t index) { return buffer[index]; }
    const int& operator[](size_t index) const { return buffer[index]; }

    size_t size() const { return buffer.size(); }
};
struct StanHash {
    std::size_t operator()(const Stan& s) const {
        std::size_t seed = 0;
        for (int val : s.data()) {
            // kombajn do haszy
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
    

    explicit BfsBaza(const Stan& s, const Stan& p, HashMap& moja, const HashMap& obca) 
                : pojemnosc(p), mapa(moja), mapaOther(obca) {
        
        kol.reserve(rozmiarPamieci);

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

int solve(const std::vector<int>& x, const std::vector<int>& y) {
	Stan pojemnosc(x), koniec(y), wyzerowany(n);

    std::unordered_map<Stan, int, StanHash> mapaDolu, mapaGory;
    mapaDolu.reserve(rozmiarPamieci);
    mapaGory.reserve(rozmiarPamieci);

    BfsDol dol(wyzerowany, pojemnosc, mapaDolu, mapaGory);
    BfsGora gora(koniec, pojemnosc, mapaGory, mapaDolu);

	while(!dol.czyKolejkaPusta() && !gora.czyKolejkaPusta()) {
        
        if(auto wynik = dol.wywolajCykl())  return *wynik;
        if(mapaGory.size() <= rozmiarPamieci)
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

    // to ma szansę skasować dużo stanów
    int nwdX = 0;
    for(int i = 0; i < n; i++) {
        nwdX = std::gcd(nwdX, pojemnosc[i]);
    }
    for(int i = 0; i < n; i++) {
        pojemnosc[i] /= nwdX;
        koniec[i] /= nwdX;
    }

    long long liczbaMozliwychStanow = 1;
    for(int i = 0; i < n; i++) {
        liczbaMozliwychStanow *= pojemnosc[i] + 1;
        if(liczbaMozliwychStanow > 10'000'000)
            break;
    }
    rozmiarPamieci = liczbaMozliwychStanow;

	std::cout << solve(pojemnosc, koniec) << "\n";
	return 0;
}