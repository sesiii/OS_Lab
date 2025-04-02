run: LRU.c
	gcc -Wall -o runsearch LRU.c 
	./runsearch > output.txt
	
vrun: LRU.c
	gcc -Wall -DVERBOSE -o runsearch LRU.c
	./runsearch > verbose_output.txt
	
clean:
	rm -f runsearch