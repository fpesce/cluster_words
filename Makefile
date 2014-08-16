# CFLAGS = -O3 -Wall -fprofile-generate / -fprofile-use
CFLAGS = -Wall -O0 -ggdb

INCLUDE = -Isrc/

LIBS = -lm

MODULES = src/cluster_words.c mmap_wrapper.o levenshtein.o heap.o hash.o list.o

TARGET = cluster_words

all: $(TARGET)

clean:
	rm -f *.o
	rm -f src/*~
	rm -f $(TARGET)

hash.o: src/hash.c
	$(CC) $(CFLAGS) $(INCLUDE) -c src/hash.c

list.o: src/list.c
	$(CC) $(CFLAGS) $(INCLUDE) -c src/list.c

heap.o: src/heap.c
	$(CC) $(CFLAGS) $(INCLUDE) -c src/heap.c

mmap_wrapper.o: src/mmap_wrapper.c
	$(CC) $(CFLAGS) $(INCLUDE) -c src/mmap_wrapper.c

levenshtein.o: src/levenshtein.c
	$(CC) $(CFLAGS) $(INCLUDE) -c src/levenshtein.c

$(TARGET): $(MODULES)
	$(CC) $(CFLAGS) $(INCLUDE) $(MODULES) -o $(TARGET) $(LIBS)
