all: parent child dummy run

parent: parent.c
		gcc -o parent parent.c

child: child.c
		gcc -o child child.c

dummy: dummy.c
		gcc -o dummy dummy.c

run: parent
		./parent 15 

clean:
		rm -f parent child dummy