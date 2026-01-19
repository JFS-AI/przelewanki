prz.e: prz.cpp
	g++ @opcjeCpp prz.cpp -lm -o prz.e

szybko: prz.cpp
	g++ prz.cpp -o prz.e -O3 -static

clear:
	rm *.e
