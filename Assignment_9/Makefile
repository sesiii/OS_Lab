run: demandpaging.c
	gcc -Wall -o runsearch demandpaging.c 
	./runsearch > output.txt
	
vrun: demandpaging.c
	gcc -Wall -DVERBOSE -o runsearch demandpaging.c
	./runsearch > verbose_output.txt
	
clean:
	rm -f runsearch