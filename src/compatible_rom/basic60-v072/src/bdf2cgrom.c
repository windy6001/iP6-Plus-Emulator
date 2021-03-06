/*
  bdf2cgrom.c  2019.9.17 version
    convert .bdf font file to CGROM format for PC-6001
    by AKIKAWA, Hisashi
    This software is redistributable under the LGPLv2.1 or any later version.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NCHR 256
#define SIZEX_DEFAULT 8
#define SIZEY_DEFAULT 12

int main(int argc, char *argv[])
{
  int i, j;
  int opt;
  int ascii = 0;
  int count = 0;
  int code = 0, chr;
  int x, y;
  int ascent = -1, descent = -1, pixel_x, pixel_y;
  int x1, x2, xmin = 999, xmax = -1;
  int size_x_out = SIZEX_DEFAULT, size_y_out = SIZEY_DEFAULT;
  int size_x, size_y;
  int offset_x, offset_y;
  double ratio_x, ratio_y;
  char line[100];
  char charset[100] = "";
  char outfile[FILENAME_MAX + 1];
  unsigned char bmpbdf[64][64];
  unsigned char bmp60[NCHR][16];
  long long data;
  FILE *fp;

  int *tbl = NULL, *altsuit;
  int jistbl[] = {
    0x2121, 0x376e, 0x3250, 0x3f65, 0x4c5a, 0x3662, 0x455a, 0x467c,
    0x472f, 0x315f, 0x3b7e, 0x4a2c, 0x4943, 0x4934, 0x4069, 0x4b7c,
    0x2650, 0x2835, 0x2833, 0x2834, 0x2832, 0x2836, 0x282d, 0x282c,
    0x282e, 0x282f, 0x2831, 0x2830, 0x215f, 0x4267, 0x4366, 0x3e2e,
    0x2121, 0x212a, 0x2149, 0x2174, 0x2170, 0x2173, 0x2175, 0x2147,
    0x214a, 0x214b, 0x2176, 0x215c, 0x2124, 0x215d, 0x2125, 0x213f,
    0x2330, 0x2331, 0x2332, 0x2333, 0x2334, 0x2335, 0x2336, 0x2337,
    0x2338, 0x2339, 0x2127, 0x2128, 0x2163, 0x2161, 0x2164, 0x2129,
    0x2177, 0x2341, 0x2342, 0x2343, 0x2344, 0x2345, 0x2346, 0x2347,
    0x2348, 0x2349, 0x234a, 0x234b, 0x234c, 0x234d, 0x234e, 0x234f,
    0x2350, 0x2351, 0x2352, 0x2353, 0x2354, 0x2355, 0x2356, 0x2357,
    0x2358, 0x2359, 0x235a, 0x214c, 0x216f, 0x214d, 0x2130, 0x2132,
    0x2121, 0x2361, 0x2362, 0x2363, 0x2364, 0x2365, 0x2366, 0x2367,
    0x2368, 0x2369, 0x236a, 0x236b, 0x236c, 0x236d, 0x236e, 0x236f,
    0x2370, 0x2371, 0x2372, 0x2373, 0x2374, 0x2375, 0x2376, 0x2377,
    0x2378, 0x2379, 0x237a, 0x2150, 0x2143, 0x2151, 0x2141, 0x2121,
    0x263a, 0x263e, 0x2640, 0x263c, 0x217b, 0x217c, 0x2472, 0x2421,
    0x2423, 0x2425, 0x2427, 0x2429, 0x2463, 0x2465, 0x2467, 0x2443,
    0x2121, 0x2422, 0x2424, 0x2426, 0x2428, 0x242a, 0x242b, 0x242d,
    0x242f, 0x2431, 0x2433, 0x2435, 0x2437, 0x2439, 0x243b, 0x243d,
    0x2121, 0x2123, 0x2156, 0x2157, 0x2122, 0x2126, 0x2572, 0x2521,
    0x2523, 0x2525, 0x2527, 0x2529, 0x2563, 0x2565, 0x2567, 0x2543,
    0x213c, 0x2522, 0x2524, 0x2526, 0x2528, 0x252a, 0x252b, 0x252d,
    0x252f, 0x2531, 0x2533, 0x2535, 0x2537, 0x2539, 0x253b, 0x253d,
    0x253f, 0x2541, 0x2544, 0x2546, 0x2548, 0x254a, 0x254b, 0x254c,
    0x254d, 0x254e, 0x254f, 0x2552, 0x2555, 0x2558, 0x255b, 0x255e,
    0x255f, 0x2560, 0x2561, 0x2562, 0x2564, 0x2566, 0x2568, 0x2569,
    0x256a, 0x256b, 0x256c, 0x256d, 0x256f, 0x2573, 0x212b, 0x212c,
    0x243f, 0x2441, 0x2444, 0x2446, 0x2448, 0x244a, 0x244b, 0x244c,
    0x244d, 0x244e, 0x244f, 0x2452, 0x2455, 0x2458, 0x245b, 0x245e,
    0x245f, 0x2460, 0x2461, 0x2462, 0x2464, 0x2466, 0x2468, 0x2469,
    0x246a, 0x246b, 0x246c, 0x246d, 0x246f, 0x2473, 0x2121, 0x2121
  };
  int unitbl[] = {
    0x3000, 0x6708, 0x706b, 0x6c34, 0x6728, 0x91d1, 0x571f, 0x65e5,
    0x5e74, 0x5186, 0x6642, 0x5206, 0x79d2, 0x767e, 0x5343, 0x4e07,
    0x03c0, 0x253b, 0x2533, 0x252b, 0x2523, 0x254b, 0x2503, 0x2501,
    0x250f, 0x2513, 0x2517, 0x251b, 0x00d7, 0x5927, 0x4e2d, 0x5c0f,
    0x3000, 0xff01, 0x201d, 0xff03, 0xff04, 0xff05, 0xff06, 0x2019,
    0xff08, 0xff09, 0xff0a, 0xff0b, 0xff0c, 0xff0d, 0xff0e, 0xff0f,
    0xff10, 0xff11, 0xff12, 0xff13, 0xff14, 0xff15, 0xff16, 0xff17,
    0xff18, 0xff19, 0xff1a, 0xff1b, 0xff1c, 0xff1d, 0xff1e, 0xff1f,
    0xff20, 0xff21, 0xff22, 0xff23, 0xff24, 0xff25, 0xff26, 0xff27,
    0xff28, 0xff29, 0xff2a, 0xff2b, 0xff2c, 0xff2d, 0xff2e, 0xff2f,
    0xff30, 0xff31, 0xff32, 0xff33, 0xff34, 0xff35, 0xff36, 0xff37,
    0xff38, 0xff39, 0xff3a, 0x3014, 0xffe5, 0x3015, 0xff3e, 0xff3f,
    0x3000, 0xff41, 0xff42, 0xff43, 0xff44, 0xff45, 0xff46, 0xff47,
    0xff48, 0xff49, 0xff4a, 0xff4b, 0xff4c, 0xff4d, 0xff4e, 0xff4f,
    0xff50, 0xff51, 0xff52, 0xff53, 0xff54, 0xff55, 0xff56, 0xff57,
    0xff58, 0xff59, 0xff5a, 0xff5b, 0xff5c, 0xff5d, 0x301c, 0x3000,
    0x2660, 0x2665, 0x2663, 0x2666, 0x25cb, 0x25cf, 0x3092, 0x3041,
    0x3043, 0x3045, 0x3047, 0x3049, 0x3083, 0x3085, 0x3087, 0x3063,
    0x3000, 0x3042, 0x3044, 0x3046, 0x3048, 0x304a, 0x304b, 0x304d,
    0x304f, 0x3051, 0x3053, 0x3055, 0x3057, 0x3059, 0x305b, 0x305d,
    0x3000, 0x3002, 0x300c, 0x300d, 0x3001, 0x30fb, 0x30f2, 0x30a1,
    0x30a3, 0x30a5, 0x30a7, 0x30a9, 0x30e3, 0x30e5, 0x30e7, 0x30c3,
    0x30fc, 0x30a2, 0x30a4, 0x30a6, 0x30a8, 0x30aa, 0x30ab, 0x30ad,
    0x30af, 0x30b1, 0x30b3, 0x30b5, 0x30b7, 0x30b9, 0x30bb, 0x30bd,
    0x30bf, 0x30c1, 0x30c4, 0x30c6, 0x30c8, 0x30ca, 0x30cb, 0x30cc,
    0x30cd, 0x30ce, 0x30cf, 0x30d2, 0x30d5, 0x30d8, 0x30db, 0x30de,
    0x30df, 0x30e0, 0x30e1, 0x30e2, 0x30e4, 0x30e6, 0x30e8, 0x30e9,
    0x30ea, 0x30eb, 0x30ec, 0x30ed, 0x30ef, 0x30f3, 0x309b, 0x309c,
    0x305f, 0x3061, 0x3064, 0x3066, 0x3068, 0x306a, 0x306b, 0x306c,
    0x306d, 0x306e, 0x306f, 0x3072, 0x3075, 0x3078, 0x307b, 0x307e,
    0x307f, 0x3080, 0x3081, 0x3082, 0x3084, 0x3086, 0x3088, 0x3089,
    0x308a, 0x308b, 0x308c, 0x308d, 0x308f, 0x3093, 0x3000, 0x3000
  };

  int jisaltsuit[] = {0x2225, 0x2227, 0x217a, 0x2221};
  int unialtsuit[] = {0x25b2, 0x25bc, 0x2605, 0x25c6};

  for (opt = 1; argc > opt && argv[opt][0] == '-'; opt++) {
    if (atoi(argv[opt]) == -8) {
      size_y_out = 8;
    } else if (atoi(argv[opt]) == -10) {
      size_y_out = 10;
    } else if (atoi(argv[opt]) == -12) {
      size_y_out = 12;
    } else if (strcmp(argv[opt], "-ascii") == 0) {
      ascii = 1;
    } else {
      printf("output height must be 8, 10, or 12\n");
      exit(1);
    }
  }

  if (argc < opt + 2) {
    printf("usage: bdf2cgrom [-8] [-10] [-12] [-ascii] fontfile.bdf outfile\n");
    exit(1);
  }

  if (strcmp(argv[opt], "-") == 0) {
    fp = stdin;
  } else {
    fp = fopen(argv[opt], "rb");
    if (fp == NULL) {
      printf("cannot open %s\n", argv[opt]);
      exit(1);
    }
  }

  memset(bmp60, 0, sizeof(bmp60));
  while (fgets(line, sizeof(line), fp)) {
    sscanf(line, "ENCODING %d", &code);
    x1 = x2 = 0;
    if (sscanf(line, "BBX %d %d %d %d", &x1, &i, &x2, &i)) {
      if (tbl != unitbl || (code != 0x2e3a && code != 0x2e3b)) {
	if (x1 < xmin) xmin = x1;
	if (x2 < xmin) xmin = x2;
	if (x1 > xmax) xmax = x1;
	if (x2 > xmax) xmax = x2;
      }
    }
    sscanf(line, "FONT_ASCENT %d", &ascent);
    sscanf(line, "FONT_DESCENT %d", &descent);
    if (sscanf(line, "CHARSET_REGISTRY %s", charset) == 1) {
      if (strncasecmp(charset, "\"JISX0208", 9) == 0 ||
	  strncasecmp(charset, "\"JISX0213", 9) == 0) {
	tbl = jistbl;
	altsuit = jisaltsuit;
      } else if (strcasecmp(charset, "\"ISO10646\"") == 0) {
	tbl = unitbl;
	altsuit = unialtsuit;
      } else if (!ascii) {
	printf("cannot convert for charset %s\n", charset);
	exit(1);
      }
    }
  }
  pixel_x = xmax - xmin;
  pixel_y = ascent + descent;
  if (ascent < 0 || descent < 0 || pixel_x < 0 || strlen(charset) == 0) {
    printf("illegal bdf\n");
    exit(1);
  }
  rewind(fp);

  while (count < NCHR + 4) {
    if (fgets(line, sizeof(line), fp) == NULL) {
      break;
    }
    chr = -1;
    if (sscanf(line, "ENCODING %d", &code) == 1) {
      for (i = 0; i < NCHR; i++) {
	if ((ascii && code == i) || (!ascii && code == tbl[i])) {
	  chr = i;
	  count++;
	  break;
	}
      }

      for (i = 0; i < 4; i++) {
	if (code == altsuit[i]) {
	  chr = 0x80 + i;
	  j = 0;
	  for (i = 0; i < size_y_out; i++) {
	    j |= bmp60[chr][i];
	  }
	  if (j) {
	    chr = -1;
	  } else {
	    count++;
	  }
	  break;
	}
      }
    }

    if (chr != -1) {
      memset(bmpbdf, 0, sizeof(bmpbdf));
      do {
	fgets(line, sizeof(line), fp);
      } while (sscanf(line, "BBX %d %d %d %d", &size_x, &size_y, &offset_x, &offset_y) != 4);

      if (pixel_x > size_x_out) {
	ratio_x = (double)pixel_x / size_x_out;
      } else {
	ratio_x = 1;
	offset_x += (size_x_out - pixel_x) / 2;
      }
      if (pixel_y > size_y_out) {
	ratio_y = (double)pixel_y / size_y_out;
      } else {
	ratio_y = 1;
	offset_y -= (size_y_out - pixel_y) / 2;
      }

      if (offset_x < 0) {
	offset_x = 0;
      }

      do {
	fgets(line, sizeof(line), fp);
      } while (strncmp(line, "BITMAP", 6) != 0);

      for (i = 0; i < size_y; i++) {
	fgets(line, sizeof(line), fp);
#ifdef WIN32
	while (sscanf(line, "%I64x", &data) != 1) {
#else
	while (sscanf(line, "%llx", &data) != 1) {
#endif
	  ;
	}
	for (j = 0; j < size_x; j++) {
	  x = j + offset_x;
	  y = i + ascent - size_y - offset_y;
	  bmpbdf[x][y] = (data >> ((64 - size_x) % 8 + size_x - j - 1)) & 1;
	}
      }

      if (size_y > 0) {
	for (i = 0; i < size_y_out; i++) {
	  bmp60[chr][i] = 0;
	  for (j = 0; j < size_x_out; j++) {
	    data = 0;
	    for (x = j * ratio_x; x < (int)((j + 1) * ratio_x + .01); x++) {
	      for (y = i * ratio_y; y < (int)((i + 1) * ratio_y + .01); y++) {
		data += bmpbdf[x][y];
	      }
	    }
	    if ((double)data / (x - (int)(j * ratio_x)) * (y - (int)(i * ratio_y)) > 0) {
	      bmp60[chr][i] += 1 << (size_x_out - j - 1);
	    }
	  }
	}
      }
    }
  }
  fclose(fp);

  strncpy(outfile, argv[opt + 1], sizeof(outfile));
  if (strcmp(outfile, "-") == 0) {
    fp = stdout;
  } else {
    fp = fopen(outfile, "wb");
    if (fp == NULL) {
      printf("cannot open %s\n", argv[opt + 1]);
      exit(1);
    }
  }
  for (i = 0; i < NCHR; i++) {
    for (j = 0; j < 16; j++) {
      fputc(bmp60[i][j], fp);
    }
  }
  fclose(fp);

  return 0;
}
