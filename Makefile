TESTS := in1 in2 in3

pratt: pratt.c
	gcc $< -g -Wall -D_GNU_SOURCE -o $@

test: $(TESTS)

$(TESTS): pratt
	@./$< < $@

clean:
	$(RM) pratt

.PHONY: clean $(TESTS)
