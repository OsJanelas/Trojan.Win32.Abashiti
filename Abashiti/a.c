main(t) {
  for (t = 0;; t++) {
    // A mágica acontece na combinação de senos e modulação de frequência
    // O valor de saída (char) cria uma forma de onda ruidosa
    putchar(
      (t * (t >> (t >> 13 & 7) & (t >> 10 & 15))) | // Basic structure
      (t * (t >> 12 & t >> 8)) ^                    // Scream
      (t >> 4 & t >> 7)                             // Crying
    );
  }
}