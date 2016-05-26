
EXE=lpc-uart-calc

$(EXE): main.c
	gcc $^ -lm -o $(EXE)

clean:
	rm -f $(EXE)
