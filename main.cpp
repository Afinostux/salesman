
#ifdef _WIN32
#define PNAME "salesman.exe"
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <ctime>
#else
#define PNAME "salesman"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#endif


typedef uint32_t u32;

#define FILEMAGIC "FOXSELLS"

float frand(void)
{
   return ((float)rand())/RAND_MAX;
}

float frandrange(float min, float max)
{
   return (frand() * (max - min)) + min;
}

struct tssheader {
   char magic[8];
   u32 version;
   u32 point_count, point_ofs;
   u32 solution_ofs;
};

struct point {
   float x, y;
};

float distance(point * a, point * b)
{
   float dx = a->x - b->x;
   float dy = a->y - b->y;
   float sqd = (dx * dx) + (dy * dy);
   return sqrtf(sqd);
}

u32 factorial(u32 setsize)
{
   if (setsize) {
      u32 accum = setsize;
      setsize -= 1;
      while (setsize) {
         accum *= setsize;
         setsize -= 1;
      }
      return accum;
   }
   return 1;
}

#define MAKESET_S "--makeset"

void usage(void)
{
   printf(PNAME " - a traveling salesman solver\n");
   printf("by Austin Fox (fostinaux@gmail.com)\n");
   printf("usage:\n");
   printf(PNAME " " MAKESET_S " NAME W H N\n");
   printf("  make a new dataset named NAME, width of W, height of H, and N points\n");
}

void print_visitlist(u32 * visitlist, u32 n)
{
   for (u32 i = 0; i < n; i++) {
      printf("%u ", visitlist[i]);
   }
   printf("\n");
}

void plainchange(u32 * visitlist, u32 n, u32 step)
{
   u32 a = 0;
   u32 b = 0;
   u32 phase = step/n;
   u32 modstep = step%n;
   if (phase%2) {
      if (modstep) {
         a = (n - modstep) - 1;
         b = a + 1;
      } else {
         a = 0;
         b = 1;
      }
   } else {
      if (modstep) {
         a = modstep - 1;
         b = a + 1;
      } else {
         a = n - 2;
         b = n - 1;
      }
   }
   u32 temp = visitlist[b];
   visitlist[b] = visitlist[a];
   visitlist[a] = temp;
   //printf("plainchange [%u, %u]\n", a, b);
}

float path_distance(point * points, u32 * visitlist, u32 n)
{
   float dist = 0;
   for (u32 i = 0; i < n - 1; i++) {
      u32 il = visitlist[i];
      dist += distance(points + il, points + il + 1);
   }
   dist += distance(points, points + (n - 1));
   return dist;
}

void copy_visitlist(u32 * dst, const u32 * src, u32 n)
{
   for (u32 i = 0; i < n; i++) {
      printf("copying %u\n", i);
      dst[i] = src[i];
   }
}

// TODO(afox): only works when set to 10?
#define SLICE_SIZE 10
void bruteforce(point * points, u32 * visitlist, u32 n)
{
   u32 fcount = factorial(n);
   printf("making brute force solution (%u points, %u permutations)\n",
         n, fcount);
   u32 percent_accum = 0;
   u32 percent_slice = fcount / SLICE_SIZE;
   u32 * templist = (u32*)malloc(sizeof(u32) * n);
   for (u32 i = 0; i < n; i++) {
      templist[i] = visitlist[i] = i;
   }
   float best_distance = path_distance(points, visitlist, n);
   printf("starting distance = %f\n", best_distance);
   for (u32 i = 0; i < fcount; i++) {
      printf("iteration %u\n", i);
      plainchange(templist, n, i);
      printf("plainchange done\n");
      float test_distance = path_distance(points, templist, n);
      printf("test distance done\n");
#if 1
      if (test_distance < best_distance) {
         copy_visitlist(visitlist, templist, n);
         printf("memcopy done\n");
         best_distance = test_distance;
         printf("best distance = %f\n", best_distance);
      }
#endif
      while (i / percent_slice > percent_accum) {
         percent_accum += 1;
         printf(" [%u%%]\n", percent_accum * SLICE_SIZE);
      }
   }
   free(templist);
   printf(" [100%%]\n");
}

void makeset(const char* name, const char* wst, const char* hst, const char* nst)
{
   printf("makeset called: %s %s %s %s\n", name, wst, hst, nst);
   float w = atof(wst);
   float h = atof(hst);
   int n = atoi(nst);
   if (w <= 0 || h <= 0 || n <= 0) {
      usage();
      return;
   }
   tssheader head = {};
   memcpy(head.magic, FILEMAGIC, 8);
   head.version = 1;
   head.point_count = n;
   head.point_ofs = sizeof(tssheader);
   head.solution_ofs = head.point_ofs + (sizeof(point) * n);
   point * points = (point*)malloc(sizeof(point)*n);
   float w_max = w * 0.5;
   float h_max = h * 0.5;
   float w_min = -w_max;
   float h_min = -h_max;
   u32 * visitlist = (u32*)malloc(sizeof(u32)*n);
   for (int i = 0; i < n; i++) {
      point * cp = points + i;
      cp->x = frandrange(w_min, w_max);
      cp->y = frandrange(h_min, h_max);
   }
   bruteforce(points, visitlist, n);
   FILE * f = fopen(name, "wb");
   fwrite(&head, sizeof(tssheader), 1, f);
   fwrite(points, sizeof(point), n, f);
   fwrite(visitlist, sizeof(u32), n, f);
   fclose(f);
   free(visitlist);
   free(points);
}

int main(int argc, const char ** argv)
{
   srand(time(NULL));
   printf("start\n");
   if (argc == 6) {
      printf("argc\n");
      if (!strcmp(argv[1], MAKESET_S)) {
         printf("scmp\n");
         makeset(argv[2], argv[3], argv[4], argv[5]);
         return 0;
      }
   }
   usage();
   return 0;
}
