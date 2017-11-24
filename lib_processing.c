#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "imageprocessing.h"
#include "brightness.h"

#include <FreeImage.h>

/*
  imagem abrir_imagem(char *nome_do_arquivo);
  void salvar_imagem(char *nome_do_arquivo);
  void liberar_imagem(imagem *i);
*/

int debug = 1;
void dbgmsg(const char *s, ...) {
	if (debug) {
		va_list args;
		va_start(args, s);
		vprintf(s, args);
		va_end(args);
	}
}

imagem abrir_imagem(char *nome_do_arquivo) {
	FIBITMAP *bitmapIn;
	int x, y;
	RGBQUAD color;
	imagem I;

	bitmapIn = FreeImage_Load(FIF_JPEG, nome_do_arquivo, 0);

	if (bitmapIn == 0) {
		fprintf(stderr, "Erro! Nao achei arquivo - %s\n", nome_do_arquivo);
	} else {
		dbgmsg("Arquivo lido corretamente!\n");
	}

	x = FreeImage_GetWidth(bitmapIn);
	y = FreeImage_GetHeight(bitmapIn);

	I.width = x;
	I.height = y;
	I.r = (float*) mmap(NULL, sizeof(float) * x * y, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
	I.g = (float*) mmap(NULL, sizeof(float) * x * y, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
	I.b = (float*) mmap(NULL, sizeof(float) * x * y, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);

	for (int i=0; i<x; i++) {
		for (int j=0; j <y; j++) {
			int idx;
			FreeImage_GetPixelColor(bitmapIn, i, j, &color);

			idx = i + (j*x);

			I.r[idx] = color.rgbRed;
			I.g[idx] = color.rgbGreen;
			I.b[idx] = color.rgbBlue;
		}
	}
	FreeImage_Unload(bitmapIn);
	return I;
}

float pixel_max(imagem *I)
{
	int img_size = I->height * I->width;
	float  pixval = 0, mean_square = 0, max = 0;
	for (int index = 0; index < img_size; index++) {
		pixval = I->r[index];
		mean_square = pixval * pixval;
		pixval = I->g[index];
		mean_square += pixval * pixval;
		pixval = I->b[index];
		mean_square += pixval * pixval;
		if (mean_square > max) {
			max = mean_square;
		}
	}
	return max;
}

void liberar_imagem(imagem *I) {
	munmap(I->r, I->height * I->width * sizeof(float));
	munmap(I->g, I->height * I->width * sizeof(float));
	munmap(I->b, I->height * I->width * sizeof(float));
}

void aplicar_brilho(float brilho, imagem *I) {
	switch (strategy) {
	case fork_lines:
		apply_bright_fork_lines(brilho, I, num_jobs);
		break;

	case fork_columns:
		apply_bright_fork_columns(brilho, I, num_jobs);
		break;

	case thread_lines:
		apply_bright_thread_lines(brilho, I, num_jobs);
		break;

	case thread_columns:
		apply_bright_thread_columns(brilho, I, num_jobs);
		break;

	case single_thread_lines:
		apply_bright_single_thread_lines(brilho, I);
		break;

	case single_thread_columns:
		
		break;

	default:
		break;
	}
}

void salvar_imagem(char *nome_do_arquivo, imagem *I) {
	FIBITMAP *bitmapOut;
	RGBQUAD color;

	dbgmsg("Salvando imagem %d por %d...\n", I->width, I->height);
	bitmapOut = FreeImage_Allocate(I->width, I->height, 24, 0, 0, 0);

	for (int i=0; i<I->width; i++) {
		for (int j=0; j<I->height; j++) {
			int idx;

			idx = i + (j*I->width);
			color.rgbRed = I->r[idx];
			color.rgbGreen = I->g[idx];
			color.rgbBlue = I->b[idx];

			FreeImage_SetPixelColor(bitmapOut, i, j, &color);
		}
	}

	FreeImage_Save(FIF_JPEG, bitmapOut, nome_do_arquivo, JPEG_DEFAULT);
	FreeImage_Unload(bitmapOut);
}
