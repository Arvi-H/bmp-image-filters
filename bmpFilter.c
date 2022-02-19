#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE  1
#define FALSE 0

#define BAD_NUMBER_ARGS 1
#define FSEEK_ERROR 2
#define FREAD_ERROR 3
#define MALLOC_ERROR 4
#define FWRITE_ERROR 5

/**
 * Parses the command line.
 *
 * argc:      the number of items on the command line (and length of the
 *            argv array) including the executable
 * argv:      the array of arguments as strings (char* array)
 * grayscale: the integer value is set to TRUE if grayscale output indicated
 *            outherwise FALSE for threshold output
 *
 * returns the input file pointer (FILE*)
 **/
FILE *parseCommandLine(int argc, char **argv, int *isGrayscale) {
  if (argc > 2) {
    printf("Usage: %s [-g]\n", argv[0]);
    exit(BAD_NUMBER_ARGS);
  }
  
  if (argc == 2 && strcmp(argv[1], "-g") == 0) {
    *isGrayscale = TRUE;
  } else {
    *isGrayscale = FALSE;
  }

  return stdin;
}

unsigned getFileSizeInBytes(FILE* stream) {
  unsigned fileSizeInBytes = 0;
  
  rewind(stream);
  if (fseek(stream, 0L, SEEK_END) != 0) {
    exit(FSEEK_ERROR);
  }
  fileSizeInBytes = ftell(stream);

  return fileSizeInBytes;
}

void getBmpFileAsBytes(unsigned char* ptr, unsigned fileSizeInBytes, FILE* stream) {
  rewind(stream);
  if (fread(ptr, fileSizeInBytes, 1, stream) != 1) {
#ifdef DEBUG
    printf("feof() = %x\n", feof(stream));
    printf("ferror() = %x\n", ferror(stream));
#endif
    exit(FREAD_ERROR);
  }
}

unsigned char getAverageIntensity(unsigned char blue, unsigned char green, unsigned char red) {
	unsigned char averageIntensity = (blue + green + red) / 3;
	return averageIntensity;
}

void applyGrayscaleToPixel(unsigned char* pixel) {

}

/**  Pixels in the original image with an average intensity of 128 or more will become white while
	those with average intensities below 128 will become black.
*/
void applyThresholdToPixel(unsigned char* pixel) {
	unsigned char red = pixel[0];
	unsigned char green = pixel[1];
	unsigned char blue = pixel[2];
	unsigned char averageIntesity = getAverageIntensity(blue, green, red);

	if (averageIntesity >= 128) {
		for (int i = 0; i < 3; i++) {
			pixel[i] = 0xff;
		}
	} else {
		for (int i = 0; i < 3; i++) {
			pixel[i] = 0x00;
		}
	}
}

void applyFilterToPixel(unsigned char* pixel, int isGrayscale) {
	if (isGrayscale == TRUE) {
		applyGrayscaleToPixel(pixel);
	} else {
		applyThresholdToPixel(pixel);
	}
}

void applyFilterToRow(unsigned char* row, int width, int isGrayscale) {
	// applyFilterToPixel to every pixel in width long row	
	for (int i = 0; i < width; i++) {
		applyFilterToPixel((row+i), isGrayscale);
	}
}

void applyFilterToPixelArray(unsigned char* pixelArray, int width, int height, int isGrayscale) {
	// 3 bytes per pixel
	int widthInBytes = width * 3; 
	// Each row will occupy a multiple of 4 bytes - You can change this to whatever though
	int paddingMultiple = 4;
	// Padding between each row
	int padding = paddingMultiple - (widthInBytes % paddingMultiple);

	// Loop through the entire file and call applyFilterToRow on every row
	for (int i = 0; i < height; i++) {
		// Row begins at pizelArray and is width + padding big. 
		unsigned char* row = pixelArray + widthInBytes;
		applyFilterToRow(row, width, isGrayscale);	
		// Padding only after the first row.
		row += padding;
	}
}

void parseHeaderAndApplyFilter(unsigned char* bmpFileAsBytes, int isGrayscale) {
	int offsetFirstBytePixelArray = *((int*) (bmpFileAsBytes+10));

	// We are getting this information from BITMAPINFOHEADER 
	int width = *((int*) (bmpFileAsBytes+18));
	int height = *((int*) (bmpFileAsBytes+22));

	unsigned char* pixelArray = (bmpFileAsBytes + 54);

	printf("offsetFirstBytePixelArray = %u\n", offsetFirstBytePixelArray);
	printf("width = %u\n", width);
	printf("height = %u\n", height);
	printf("pixelArray = %p\n", pixelArray);

	applyFilterToPixelArray(pixelArray, width, height, isGrayscale);
}

int main(int argc, char **argv) {
	int grayscale = FALSE;
	unsigned fileSizeInBytes = 0;
	unsigned char* bmpFileAsBytes = NULL;
	FILE *stream = NULL;
	
	stream = parseCommandLine(argc, argv, &grayscale);
	fileSizeInBytes = getFileSizeInBytes(stream);

#ifdef DEBUG
  	printf("fileSizeInBytes = %u\n", fileSizeInBytes);
#endif

	bmpFileAsBytes = (unsigned char *)malloc(fileSizeInBytes);
	if (bmpFileAsBytes == NULL) {
		exit(MALLOC_ERROR);
	}
	getBmpFileAsBytes(bmpFileAsBytes, fileSizeInBytes, stream);

	parseHeaderAndApplyFilter(bmpFileAsBytes, grayscale);

#ifndef DEBUG
	if (fwrite(bmpFileAsBytes, fileSizeInBytes, 1, stdout) != 1) {
		exit(FWRITE_ERROR);
	}
#endif

	free(bmpFileAsBytes);
	return 0;
}
