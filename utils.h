void printBits(size_t const size, void const *const ptr) {
  unsigned char *b = (unsigned char *)ptr;
  unsigned char byte;
  int i, j;

  for (i = size - 1; i >= 0; i--) {
    for (j = 7; j >= 0; j--) {
      byte = (b[i] >> j) & 1;
      printf("%u", byte);
    }
  }
  puts("");
}

// Calculate frequency from MIDI note value
// ref: https://gist.github.com/YuxiUx/ef84328d95b10d0fcbf537de77b936cd
int noteToFreq(unsigned char note) {
  float a = 440;
  return (a / 32) * pow(2, ((note - 9) / 12.0));
}