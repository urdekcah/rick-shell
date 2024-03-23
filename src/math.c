#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  int **d = (int **)malloc((len1 + 2) * sizeof(int *));
  for (int i = 0; i <= len1 + 1; i++)
    d[i] = (int *)malloc((len2 + 2) * sizeof(int)); 
  int INF = len1 + len2;
  d[0][0] = INF;
  for(int i = 0; i <= len1; i++) { d[i + 1][1] = i; d[i + 1][0] = INF; }
  for(int i = 0; i <= len2; i++) { d[1][i + 1] = i; d[0][i + 1] = INF; }
  int *sd = (int *)malloc((len1 + len2 + 1) * sizeof(int));
  memset(sd, 0, (len1 + len2 + 1) * sizeof(int));
  int *S = sd + len2;
  for (char *c = (char *)s1; *c; c++) S[(int)*c]++;
  for (char *c = (char *)s2; *c; c++) S[(int)*c]++;
  for (int i = 1; i <= len1; i++) {
    int DB = 0;
    for (int j = 1; j <= len2; j++) {
      int i1 = S[(int)s2[j - 1]], j1 = DB;
      int cost = ((s1[i - 1] == s2[j - 1]) ? 0 : 1);
      if (cost == 0) DB = j;
      d[i + 1][j + 1] = min4(
        d[i][j] + cost,
        d[i + 1][j] + 1,
        d[i][j + 1] + 1,
        d[i1][j1] + (i - i1 - 1) + 1 + (j - j1 - 1));
    }
    S[(int)s1[i - 1]] = i;
  }
  int result = d[len1 + 1][len2 + 1];
  for (int i = 0; i <= len1 + 1; i++) free(d[i]);
  free(d);
  free(sd);
  return result;
}

int similar(const char *s1, const char *s2) {
  return levenshtein_distance(s1, s2);
}