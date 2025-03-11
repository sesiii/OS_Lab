all:
	gcc -o cook cook.c -lpthread
	gcc -o customer customer.c -lpthread
	gcc -o waiter waiter.c -lpthread

db:
	gcc -Wall -o gencustomers gencustomers.c
	./gencustomers > customers.txt

clean:
	rm cook customer waiter gencustomers customers.txt
	