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

# todo -- merge test_std.c test_min.c into test.c
FILES = test.c \
aggregate_mean.o aggregate_min.o aggregate_std.o \
bench_min.o bench_mean.o bench_std.o

test: $(FILES)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o test

aggregate_min.o:
	$(CC) -c $(CFLAGS) -std=c99 -DAGGREGATE_MIN aggregate.c -o aggregate_min.o

aggregate_mean.o:
	$(CC) -c $(CFLAGS) -std=c99 -DAGGREGATE_MEAN aggregate.c -o aggregate_mean.o

aggregate_std.o:
	$(CC) -c $(CFLAGS) -std=c99 -DAGGREGATE_STD aggregate.c -o aggregate_std.o

bench_min.o:
	$(CC) -c $(CFLAGS) -DAGGREGATE_MIN bench.c -o bench_min.o

bench_mean.o:
	$(CC) -c $(CFLAGS) -DAGGREGATE_MEAN bench.c -o bench_mean.o

bench_std.o:
	$(CC) -c $(CFLAGS) -DAGGREGATE_STD bench.c -o bench_std.o
