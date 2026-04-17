anniversary2026: anniversary2026.c
	gcc -o anniversary2026 anniversary2026.c
	
all: anniversary2026

run: anniversary2026
	./anniversary2026

clean:
	rm anniversary2026
