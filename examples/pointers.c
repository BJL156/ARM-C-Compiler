void increment(int *pointer) {
  *pointer = *pointer + 1;
}

int main() {
  int value = 5;
  increment(&value);

  return value;
}
