#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *filepath) {
  FILE *file = fopen(filepath, "r");
  if (!file) {
    fprintf(stderr, "Error: Failed to open \"%s\".\n", filepath);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  rewind(file);
  char *buffer = malloc(size + 1);
  fread(buffer, 1, size, file);
  buffer[size] = '\0';

  fclose(file);
  return buffer;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <input.c> <output>\n", argv[0]);
    return 1;
  }

  char *src = read_file(argv[1]);
  if (!src) {
    return 1;
  }

  FILE *output = fopen(argv[2], "wb");
  if (!output) {
    fprintf(stderr, "Error: Failed to open \"%s\".\n", argv[2]);
    return 1;
  }

  fclose(output);

  return 0;
}
