age: age.c
	gcc -o age age.c
	
all: age

run: age
	./age

clean:
	rm age
