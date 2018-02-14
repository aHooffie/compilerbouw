%.s: %.cvc
	civcc -o $@ $<

%.o: %.s
	civas -o $@ $<

run: main.o and.o
	civvm $^

clean:
	rm -f main *.o *.s