#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "rickshell.h"
#include "rickshell/error.h"
#include "rickshell/math.h"

static inline int min(int a, int b, int c) { return a < b ? (a < c ? a : c) : (b < c ? b : c); }
static inline int min3(int a, int b, int c) { return ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c))); }
static inline int min4(int a, int b, int c, int d) { return (min3((a), (b), (c)) < (d) ? min3((a), (b), (c)) : (d)); }

int levenshtein_distance(const char *s1, const char *s2) {
  int len1 = strlen(s1), len2 = strlen(s2);
  int **d = (int **)malloc((len1 + 1) * sizeof(int *));
  for (int i = 0; i <= len1; i++)
    d[i] = (int *)malloc((len2 + 1) * sizeof(int));
  for (int i = 0; i <= len1; i++) d[i][0] = i;
  for (int j = 0; j <= len2; j++) d[0][j] = j;
  for (int i = 1; i <= len1; i++)
    for (int j = 1; j <= len2; j++) {
      int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
      d[i][j] = min(d[i-1][j] + 1, d[i][j-1] + 1, d[i-1][j-1] + cost);
    }
  int result = d[len1][len2];
  for (int i = 0; i <= len1; i++) free(d[i]);
  free(d);
  return result;
}

int damerau_levenshtein_distance(const char *s1, const char *s2) {
  int len1 = strlen(s1), len2 = strlen(s2);
  int **d = malloc((len1 + 1) * sizeof(int *));
  for (int i = 0; i <= len1; i++) d[i] = malloc((len2 + 1) * sizeof(int));
  for (int i = 0; i <= len1; i++) d[i][0] = i;
  for (int j = 0; j <= len2; j++) d[0][j] = j;
  for (int i = 1; i <= len1; i++) {
    for (int j = 1; j <= len2; j++) {
      int cost = s1[i - 1] == s2[j - 1] ? 0 : 1;
      d[i][j] = min3(d[i - 1][j] + 1, // deletion
                     d[i][j - 1] + 1, // insertion
                     d[i - 1][j - 1] + cost); // substitution
      if (i > 1 && j > 1 && s1[i - 1] == s2[j - 2] && s1[i - 2] == s2[j - 1])
        d[i][j] = min4(d[i][j], d[i - 2][j - 2] + cost, d[i - 1][j] + 1, d[i][j - 1] + 1); // transposition
    }
  }
  int result = d[len1][len2];
  for (int i = 0; i <= len1; i++)
    free(d[i]);
  free(d);
  return result;
}

int similar(const char *s1, const char *s2) {
  return damerau_levenshtein_distance(s1, s2);
}