# CFLAGS = -O3 -Wall -fprofile-generate / -fprofile-use
CFLAGS = -Wall -O0 -ggdb

INCLUDE = -Isrc/

MODULES = src/cluster_words.c mmap_wrapper.o levenshtein.o

TARGET = cluster_words

all: $(TARGET)

clean:
	rm -f *.o
	rm -f src/*~
	rm -f $(TARGET)

mmap_wrapper.o: src/mmap_wrapper.c
	$(CC) $(CFLAGS) $(INCLUDE) -c src/mmap_wrapper.c

levenshtein.o: src/levenshtein.c
	$(CC) $(CFLAGS) $(INCLUDE) -c src/levenshtein.c

$(TARGET): $(MODULES)
	$(CC) $(CFLAGS) $(INCLUDE) $(MODULES) -o $(TARGET) $(LIBS)
