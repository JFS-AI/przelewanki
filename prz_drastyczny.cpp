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

    std::unordered_map<Stan, int, VectorHash> mapa;
    mapa.reserve(1000000);
    std::deque<std::pair<Stan, int>> kolejka;
    kolejka.push(wyzerowany, 0);

	while(!kolejka.empty()) {
        auto [s, nrRuchu] = kolejka[0];

        for(int i = 0; i < n; i++) {
                
            if(s[i] != pojemnosc[i]) {
                int temp = s[i];
                s[i] = pojemnosc[i];
                if(s == koniec) return nrRuchu + 1;
                auto [it, inserted] = mapa.try_emplace(s, nrRuchu + 1);
                if(inserted) {
                    kol.emplace_back(s, nrRuchu + 1);
                }
                s[i] = temp;
            }
            
            if(s[i] > 0) {
                int temp = s[i];
                s[i] = 0;
                if(s == koniec) return nrRuchu + 1;
                auto [it, inserted] = mapa.try_emplace(s, nrRuchu + 1);
                if(inserted) {
                    kol.emplace_back(s, nrRuchu + 1);
                }
                s[i] = temp;
            }
                  
            for(int j = 0; j < n; j++) {
                if(j == i) continue;
                        
                    int przelew = std::min(s[from], pojemnosc[to] - s[to]);
                    s[to] += przelew;
                    s[from] -= przelew;
                                    
                    if(s == koniec) return nrRuchu + 1;
                    auto [it, inserted] = mapa.try_emplace(s, nrRuchu + 1);
                    if(inserted) {
                        kol.emplace_back(s, nrRuchu + 1);
                    }
                    s[to] -= przelew;
                    s[from] += przelew;
                }
            }

        kolejka.pop_front();
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