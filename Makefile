parasite:
	gcc -o parasite parasite.c
	strip parasite

clean:
	rm -rf *.o *~ parasite infected

