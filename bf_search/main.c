#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAKESTRING(n) STRING(n)
#define STRING(n) #n
typedef struct Result Result;

struct Result
{
    int cmpNum;
    int memOccupy;
    int correctWordNum;
    int totalWordNum;
};

Result result = {0, 0, 0, 0};

int buptStrcmp(const char *s1, const char *s2)
{
  while ((result.cmpNum++) && *s1 && (*s1 == *s2))
  {
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void clearSuffix(char *line)
{
  char *find = strchr(line, '\n'); //查找换行符
  if (find)                        //如果find不为空指针
    *find = '\0';
}

void *buptCalloc(size_t bytes, size_t size)
{
  if (size <= 0)
  {
    return NULL;
  }
  result.memOccupy += bytes * size;
  return calloc(bytes, size);
}

//-----------------------------------------------------------------------------
// MurmurHash2, by Austin Appleby

// Note - This code makes a few assumptions about how your machine behaves -

// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4

// And it has a few limitations -

// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.

unsigned int MurmurHash2(const void *key, int len, unsigned int seed)
{
  // 'm' and 'r' are mixing constants generated offline.
  // They're not really 'magic', they just happen to work well.

  const unsigned int m = 0x5bd1e995;
  const int r = 24;

  // Initialize the hash to a 'random' value

  unsigned int h = seed ^ len;

  // Mix 4 bytes at a time into the hash

  const unsigned char *data = (const unsigned char *)key;

  while (len >= 4)
  {
    unsigned int k = *(unsigned int *)data;

    k *= m;
    k ^= k >> r;
    k *= m;

    h *= m;
    h ^= k;

    data += 4;
    len -= 4;
  }

  // Handle the last few bytes of the input array

  switch (len)
  {
  case 3:
    h ^= data[2] << 16;
  case 2:
    h ^= data[1] << 8;
  case 1:
    h ^= data[0];
    h *= m;
  };

  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.

  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  return h;
}

struct bloom
{
  // These fields are part of the public interface of this structure.
  // Client code may read these values if desired. Client code MUST NOT
  // modify any of these.
  int entries;
  double error;
  int bits;
  int bytes;
  int hashes;

  // Fields below are private to the implementation. These may go away or
  // change incompatibly at any moment. Client code MUST NOT access or rely
  // on these.
  double bpe;
  unsigned char *bf;
  int ready;
};

int bloom_init(struct bloom *bloom, int entries, double error)
{
  bloom->ready = 0;

  if (entries < 1000 || error == 0)
  {
    return 1;
  }

  bloom->entries = entries;
  bloom->error = error;

  double num = log(bloom->error);
  double denom = 0.480453013918201; // ln(2)^2
  bloom->bpe = -(num / denom);

  double dentries = (double)entries;
  bloom->bits = (int)(dentries * bloom->bpe);

  if (bloom->bits % 8)
  {
    bloom->bytes = (bloom->bits / 8) + 1;
  }
  else
  {
    bloom->bytes = bloom->bits / 8;
  }

  bloom->hashes = (int)ceil(0.693147180559945 * bloom->bpe); // ln(2)

  bloom->bf = (unsigned char *)buptCalloc(bloom->bytes, sizeof(unsigned char));
  if (bloom->bf == NULL)
  { // LCOV_EXCL_START
    return 1;
  } // LCOV_EXCL_STOP

  bloom->ready = 1;
  return 0;
}

inline static int test_bit_set_bit(unsigned char *buf,
                                   unsigned int x, int set_bit)
{
  unsigned int byte = x >> 3;
  unsigned char c = buf[byte]; // expensive memory access
  unsigned int mask = 1 << (x % 8);

  if (c & mask)
  {
    return 1;
  }
  else
  {
    if (set_bit)
    {
      buf[byte] = c | mask;
    }
    return 0;
  }
}

static int bloom_check_add(struct bloom *bloom,
                           const void *buffer, int len, int add)
{
  if (bloom->ready == 0)
  {
    printf("bloom at %p not initialized!\n", (void *)bloom);
    return -1;
  }

  int hits = 0;
  register unsigned int a = MurmurHash2(buffer, len, 0x9747b28c);
  register unsigned int b = MurmurHash2(buffer, len, a);
  register unsigned int x;
  register unsigned int i;

  for (i = 0; i < bloom->hashes; i++)
  {
    x = (a + i * b) % bloom->bits;
    if (test_bit_set_bit(bloom->bf, x, add))
    {
      hits++;
    }
    else if (!add)
    {
      // Don't care about the presence of all the bits. Just our own.
      return 0;
    }
  }
  result.cmpNum++;
  if (hits == bloom->hashes)
  {

    return 1; // 1 == element already in (or collision)
  }

  return 0;
}

int bloom_check(struct bloom *bloom, const void *buffer, int len)
{
  return bloom_check_add(bloom, buffer, len, 0);
}

int bloom_add(struct bloom *bloom, const void *buffer, int len)
{
  return bloom_check_add(bloom, buffer, len, 1);
}

int main()
{
  struct bloom bloom;
  bloom_init(&bloom, 1270688, 0.001);
  FILE *fpPattern = fopen("patterns-127w.txt", "r");
  FILE *fpWords = fopen("words-98w.txt", "r");
  FILE *fpResults = fopen("./bf_search/result.txt", "a");
  if (fpPattern == NULL || fpWords == NULL || fpResults == NULL)
  {
    printf("File not found.");
    return 0;
  }
  char patternLine[128];
  char wordLine[128];
  while (!feof(fpPattern))
  {
    fgets(patternLine, 128, fpPattern);
    clearSuffix(patternLine);
    bloom_add(&bloom, patternLine, strlen(patternLine));
  }
  int j = 986004;
  for (int m = 0; m < j; m++) // each word
  {
    fgets(wordLine, 128, fpWords);
    clearSuffix(wordLine);
    if (bloom_check(&bloom, wordLine, strlen(wordLine)))
    {
      fprintf(fpResults, "%s", wordLine);
      fprintf(fpResults, " ");
      fprintf(fpResults, "yes\n");
      result.correctWordNum++;
    }
    else
    {
      fprintf(fpResults, "%s", wordLine);
      fprintf(fpResults, " ");
      fprintf(fpResults, "no\n");
    }
  }
  result.totalWordNum = j;
  fprintf(fpResults, "%d %d %d %d", result.memOccupy / 1024, result.cmpNum, result.totalWordNum, result.correctWordNum);
  return 0;
}