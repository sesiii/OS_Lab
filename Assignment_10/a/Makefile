run: LRU.cpp
	g++ -Wall -o runsearch LRU.cpp 
	./runsearch > output.txt
	
vrun: LRU.cpp
	g++ -Wall -DVERBOSE -o runsearch LRU.cpp
	./runsearch > verbose_output.txt
	
clean:
	rm -f runsearch