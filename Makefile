CFLAGS += -Wall -Wextra -Wpedantic

.PHONY: clean test

test: pexec
	@echo -e "\nIf you can see 'Hello World' being printed, the test is successfull\n"
	printf '#include <stdio.h>\n int main(){ puts("Hello World\\n"); return 0; }' \
	    | $(CC) $(CFLAGS) -xc - -o /dev/stdout | ./pexec

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -vf pexec
