all:
	gcc -o resource resource.c -pthread
	gcc -DNO_DEADLOCK -o resource_nodeadlock resource.c -pthread

allow: all
	./resource

avoid: all
	./resource_nodeadlock

clean:
	rm -f resource