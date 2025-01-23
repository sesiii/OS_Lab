compile: schedule.c
	gcc -Wall -o schedule schedule.c

run: compile
	./schedule > output.txt

vcompile: schedule.c
	gcc -Wall -o schedule -DVERBOSE schedule.c

vrun: vcompile
	./schedule > verbose_output.txt

db: genproc.c
	gcc -Wall -o genproc genproc.c
	./genproc > proc.txt

clean:
	-rm -f genproc schedule 