CC = gcc
CFLAGS = -g3 -Wall -Wextra -Wconversion -Wcast-qual -Wcast-align -g 
CFLAGS += -Winline -Wfloat-equal -Wnested-externs
CFLAGS += -pedantic -std=gnu99 -Werror -D_GNU_SOURCE -std=gnu99

PROMPT = -DPROMPT

EXECS = 33sh 33noprompt
.PHONY: all clean

all: $(EXECS)

33sh: sh.c jobs.c
	$(CC) -DPROMPT $(CFLAGS) $^ -o $@

33noprompt: sh.c jobs.c
	$(CC) $(CFLAGS) $^ -o $@
	
clean:
	rm -f $(EXECS)