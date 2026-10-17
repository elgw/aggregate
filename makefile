CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -lm

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

test_mean: aggregate_mean.o test_mean.c
	$(CC) $(CFLAGS) aggregate_mean.o test_mean.c $(LDFLAGS) -o test_mean

test_min: aggregate_min.o test_min.c
	$(CC) $(CFLAGS) aggregate_min.o test_min.c $(LDFLAGS) -o test_min

test_stddev: aggregate_stddev.o test.c
	$(CC) $(CFLAGS) aggregate_stddev.o test_min.c $(LDFLAGS) -o test_stddev


aggregate_min.o:
	$(CC) -c $(CFLAGS) -DAGGREGATE_MIN aggregate.c -o aggregate_min.o

aggregate_mean.o:
	$(CC) -c $(CFLAGS) -DAGGREGATE_MEAN aggregate.c -o aggregate_mean.o

aggregate_stddev.o:
	$(CC) -c $(CFLAGS) -DAGGREGATE_STDDEV aggregate.c -o aggregate_stddev.o
