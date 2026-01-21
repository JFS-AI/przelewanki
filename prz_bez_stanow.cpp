#include <iostream>
#include <numeric>
#include <algorithm>
#include <array>
#include <optional>
#include <unordered_map>
#include <deque>
#include <functional>
#include <vector>


constexpr int maxStany = 1'000'000;
int n;

using Stan = std::vector<int>;
struct VectorHash {
    std::size_t operator()(const std::vector<int>& v) const {
        std::hash<int> hasher;
        std::size_t seed = 0;
        for (int i : v) {
            // kombajn haszy z llma
            seed ^= hasher(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};


template <typename Impl>
class BfsBaza {
protected:
    const Stan pojemnosc;
    std::deque<std::pair<Stan, int>> kol;
    int nrRuchu = -1;

    void pushJesliNowy(const Stan& s) {
        auto [it, inserted] = mapa.try_emplace(s, nrRuchu + 1);
        if(inserted) {
            kol.emplace_back(s, nrRuchu + 1);
        }
    }
    std::optional<int> czyNaDrugiejMapie(const Stan& s) const {
        auto search = mapaOther.find(s);
        if(search != mapaOther.end())
            return nrRuchu + 1 + search->second;

        return std::nullopt;
    }
public:
    using HashMap = std::unordered_map<Stan, int, VectorHash>;
    HashMap& mapa;
    const HashMap& mapaOther;
    

    explicit BfsBaza(const Stan& s, const Stan& p, HashMap& moja, const HashMap& obca) 
                : pojemnosc(p), mapa(moja), mapaOther(obca) {

        pushJesliNowy(s); // potrzebujemy zacząć bfsa w jakimś punkcie
        nrRuchu++;
    }

    std::size_t sizeKolejki() const {
        return kol.size();
    }
    bool czyKolejkaPusta() const {
        return kol.empty();
    }

    std::optional<int> wywolajCykl() {
        while(!kol.empty() && kol[0].second == nrRuchu) {
            Stan s = kol[0].first;

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
            kol.pop_front();
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
    for(int val : s) {
        liczbaMozliwychStanow *= val + 1;
        if(liczbaMozliwychStanow > maxStany)
            break;
    }
    return liczbaMozliwychStanow;
}
std::size_t policzIleNowychMozeTerazPowstac(const Stan& s) {
    std::size_t liczbaMozliwychStanow = 0;
    for(int val : s) {
        liczbaMozliwychStanow += val;
        if(liczbaMozliwychStanow > maxStany)
            break;
    }
    for(int i = 0; i < n; i++)
        for(int j = 0; j < n; j++) {
            if(i == j) continue;
            liczbaMozliwychStanow += std::min(s[i], s[j]);
        }
    return liczbaMozliwychStanow;
}


int solve(const std::vector<int>& x, const std::vector<int>& y) {
	Stan pojemnosc(x), koniec(y), wyzerowany(n);

    std::size_t liczbaMozliwychStanow = policzLiczbeMozliwychStanow(pojemnosc);
    std::size_t ileMoznaNowychWygenerowac = policzIleNowychMozeTerazPowstac(pojemnosc);
    std::unordered_map<Stan, int, VectorHash> mapaDolu, mapaGory;
    mapaDolu.reserve(liczbaMozliwychStanow);
    mapaGory.reserve(liczbaMozliwychStanow);

    BfsDol dol(wyzerowany, pojemnosc, mapaDolu, mapaGory);
    BfsGora gora(koniec, pojemnosc, mapaGory, mapaDolu);

	while(!dol.czyKolejkaPusta() && !gora.czyKolejkaPusta()) {
        if(auto wynik = dol.wywolajCykl())  return *wynik;
        if(gora.sizeKolejki() * ileMoznaNowychWygenerowac <= maxStany && n >= 3)
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