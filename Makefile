%.s: %.cvc
	civcc -o $@ $<

%.o: %.s
	civas -o $@ $<

run: main.o core.o
	civvm $^

clean:
	rm -f main *.o *.s