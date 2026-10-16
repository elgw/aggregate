CFLAGS = -Wall -Wextra -Werror

OPT?=0
SAN?=0

ifeq ($(OPT),1)
CFLAGS += -O3 -DNDEBUG
LDFLAGS+=-flto
else
CFLAGS += -g3 -Og -fno-inline
ifeq ($(SAN),1)
CFLAGS += -fsanitize=address
endif
endif


test: aggregate.c test.c
	$(CC) $(CFLAGS) aggregate.c test.c $(LDFLAGS) -o test
