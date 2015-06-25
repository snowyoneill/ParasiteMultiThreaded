#include "parasite.h"

/* 
 * This file contains all methods necessary to read, write and copy bitmap image files.
 */

/// <summary>
/// Creates a new image and makes a copy of the first bitmap image.
// Second parameter is assigned to the first.
/// </summary>
int	libbmp_copyAndCreateImg(t_bmp *bmp, t_bmp *bmpCopy)
{
	bmpCopy->header.first_header.sType = bmp->header.first_header.sType;
	bmpCopy->header.first_header.iSize = bmp->header.first_header.iSize;
	bmpCopy->header.first_header.sReserved1 = bmp->header.first_header.sReserved1;
	bmpCopy->header.first_header.sReserved2 = bmp->header.first_header.sReserved2;
	bmpCopy->header.first_header.iOffBits = bmp->header.first_header.iOffBits;

	bmpCopy->header.second_header.iSize = bmp->header.second_header.iSize;
	bmpCopy->header.second_header.iWidth = bmp->header.second_header.iWidth;
	bmpCopy->header.second_header.iHeight = bmp->header.second_header.iHeight;
	bmpCopy->header.second_header.sPlanes = bmp->header.second_header.sPlanes;
	bmpCopy->header.second_header.sBitCount = bmp->header.second_header.sBitCount;
	bmpCopy->header.second_header.iCompression = bmp->header.second_header.iCompression;
	bmpCopy->header.second_header.iSizeImage = bmp->header.second_header.iSizeImage;
	bmpCopy->header.second_header.iXpelsPerMeter = bmp->header.second_header.iXpelsPerMeter;
	bmpCopy->header.second_header.iYpelsPerMeter = bmp->header.second_header.iYpelsPerMeter;
	bmpCopy->header.second_header.iClrUsed = bmp->header.second_header.iClrUsed;
	bmpCopy->header.second_header.iClrImportant = bmp->header.second_header.iClrImportant;

	bmpCopy->width = bmp->header.second_header.iWidth;
	bmpCopy->height = bmp->header.second_header.iHeight;
	bmpCopy->width_useless = bmp->width % 4;
	bmpCopy->data = (t_rgb**)malloc(bmp->height * sizeof(t_rgb*));
	if (!(bmpCopy->data))
		return (0);
	for (int i = 0; i < bmpCopy->height; i++)
	{
		bmpCopy->data[i] = (t_rgb*)malloc(bmpCopy->width * sizeof(t_rgb));
		if (!(bmpCopy->data[i]))
			return (0);
		for (int j = 0; j < bmpCopy->width; j++)
		{
			bmpCopy->data[i][j].b = bmp->data[i][j].b;
			bmpCopy->data[i][j].g = bmp->data[i][j].g;
			bmpCopy->data[i][j].r = bmp->data[i][j].r;
			bmpCopy->data[i][j].moy = bmp->data[i][j].moy;
		}
	}
	return (1);
}

/// <summary>
/// Copies only the pixel data of the first bitmap image.
/// </summary>
int	libbmp_copyImgData(t_bmp *bmp, t_bmp *bmpCopy)
{
	bmpCopy->header.first_header.sType = bmp->header.first_header.sType;
	bmpCopy->header.first_header.iSize = bmp->header.first_header.iSize;
	bmpCopy->header.first_header.sReserved1 = bmp->header.first_header.sReserved1;
	bmpCopy->header.first_header.sReserved2 = bmp->header.first_header.sReserved2;
	bmpCopy->header.first_header.iOffBits = bmp->header.first_header.iOffBits;

	bmpCopy->header.second_header.iSize = bmp->header.second_header.iSize;
	bmpCopy->header.second_header.iWidth = bmp->header.second_header.iWidth;
	bmpCopy->header.second_header.iHeight = bmp->header.second_header.iHeight;
	bmpCopy->header.second_header.sPlanes = bmp->header.second_header.sPlanes;
	bmpCopy->header.second_header.sBitCount = bmp->header.second_header.sBitCount;
	bmpCopy->header.second_header.iCompression = bmp->header.second_header.iCompression;
	bmpCopy->header.second_header.iSizeImage = bmp->header.second_header.iSizeImage;
	bmpCopy->header.second_header.iXpelsPerMeter = bmp->header.second_header.iXpelsPerMeter;
	bmpCopy->header.second_header.iYpelsPerMeter = bmp->header.second_header.iYpelsPerMeter;
	bmpCopy->header.second_header.iClrUsed = bmp->header.second_header.iClrUsed;
	bmpCopy->header.second_header.iClrImportant = bmp->header.second_header.iClrImportant;

	bmpCopy->width = bmp->header.second_header.iWidth;
	bmpCopy->height = bmp->header.second_header.iHeight;
	bmpCopy->width_useless = bmp->width % 4;

	for (int i = 0; i < bmpCopy->height; i++)
	{
		for (int j = 0; j < bmpCopy->width; j++)
		{
			bmpCopy->data[i][j].b = bmp->data[i][j].b;
			bmpCopy->data[i][j].g = bmp->data[i][j].g;
			bmpCopy->data[i][j].r = bmp->data[i][j].r;
			bmpCopy->data[i][j].moy = bmp->data[i][j].moy;
		}
	}

	return 0;
}

/// <summary>
/// Loads a bitmap image from the hard disk
/// </summary>
int	libbmp_load(char *filename, t_bmp *bmp)
{
	int		i, j;
	//FILE	*fd;
	FILE *fd = (FILE*)malloc(sizeof(t_bmp));

	if ((fd = fopen(filename, "rb")) == NULL)
		return (0);
	fread(&(bmp->header.first_header.sType), sizeof(short), 1, fd);				//  0 -  1
	fread(&(bmp->header.first_header.iSize), sizeof(int), 1, fd);				//  2 -  5
	fread(&(bmp->header.first_header.sReserved1), sizeof(short), 1, fd);		//  6 -  7
	fread(&(bmp->header.first_header.sReserved2), sizeof(short), 1, fd);		//  8 -  9
	fread(&(bmp->header.first_header.iOffBits), sizeof(int), 1, fd);			// 10 - 13

	fread(&(bmp->header.second_header.iSize), sizeof(int), 1, fd);				// 14 - 17
	fread(&(bmp->header.second_header.iWidth), sizeof(int), 1, fd);				// 18 - 21
	fread(&(bmp->header.second_header.iHeight), sizeof(int), 1, fd);			// 22 - 25
	fread(&(bmp->header.second_header.sPlanes), sizeof(short), 1, fd);			// 26 - 27
	fread(&(bmp->header.second_header.sBitCount), sizeof(short), 1, fd);		// 28 - 29
	fread(&(bmp->header.second_header.iCompression), sizeof(int), 1, fd);		// 30 - 33
	fread(&(bmp->header.second_header.iSizeImage), sizeof(int), 1, fd);			// 34 - 37
	fread(&(bmp->header.second_header.iXpelsPerMeter), sizeof(int), 1, fd);		// 38 - 41
	fread(&(bmp->header.second_header.iYpelsPerMeter), sizeof(int), 1, fd);		// 42 - 45
	fread(&(bmp->header.second_header.iClrUsed), sizeof(int), 1, fd);			// 46 - 49
	fread(&(bmp->header.second_header.iClrImportant), sizeof(int), 1, fd);		// 50 - 53

	bmp->width = bmp->header.second_header.iWidth;
	bmp->height = bmp->header.second_header.iHeight;
	bmp->width_useless = bmp->width % 4;
	bmp->data = (t_rgb**)malloc(bmp->height * sizeof(t_rgb*));
	if (!(bmp->data))
		return (0);
	for (i = 0; i < bmp->height; i++)
	{
		bmp->data[i] = (t_rgb*)malloc(bmp->width * sizeof(t_rgb));
		if (!(bmp->data[i]))
			return (0);
		for (j = 0; j < bmp->width; j++)
		{
			bmp->data[i][j].b = (unsigned char)fgetc(fd);
			bmp->data[i][j].g = (unsigned char)fgetc(fd);
			bmp->data[i][j].r = (unsigned char)fgetc(fd);
			// This line will only set the moy to 255 whenever a white pixel is loaded.
			bmp->data[i][j].moy = (unsigned char)((bmp->data[i][j].r + bmp->data[i][j].g + bmp->data[i][j].b) / 3);
		}
		for (j = 0; j < bmp->width_useless; j++)
			fgetc(fd);
	}
	fclose(fd);
	//free(fd);
	return (1);
}

/// <summary>
/// Writes a bitmap image to the hard disk
/// </summary>
int libbmp_write(char *filename, t_bmp *bmp)
{
	int		i, j;
	FILE	*fd;

	if ((fd = fopen(filename, "wb")) == NULL)
		return (0);
	fwrite(&(bmp->header.first_header.sType), sizeof(short), 1, fd);			//  0 -  1
	fwrite(&(bmp->header.first_header.iSize), sizeof(int), 1, fd);				//  2 -  5
	fwrite(&(bmp->header.first_header.sReserved1), sizeof(short), 1, fd);		//  6 -  7
	fwrite(&(bmp->header.first_header.sReserved2), sizeof(short), 1, fd);		//  8 -  9
	fwrite(&(bmp->header.first_header.iOffBits), sizeof(int), 1, fd);			// 10 - 13

	fwrite(&(bmp->header.second_header.iSize), sizeof(int), 1, fd);				// 14 - 17
	fwrite(&(bmp->header.second_header.iWidth), sizeof(int), 1, fd);			// 18 - 21
	fwrite(&(bmp->header.second_header.iHeight), sizeof(int), 1, fd);			// 22 - 25
	fwrite(&(bmp->header.second_header.sPlanes), sizeof(short), 1, fd);			// 26 - 27
	fwrite(&(bmp->header.second_header.sBitCount), sizeof(short), 1, fd);		// 28 - 29
	fwrite(&(bmp->header.second_header.iCompression), sizeof(int), 1, fd);		// 30 - 33
	fwrite(&(bmp->header.second_header.iSizeImage), sizeof(int), 1, fd);		// 34 - 37
	fwrite(&(bmp->header.second_header.iXpelsPerMeter), sizeof(int), 1, fd);	// 38 - 41
	fwrite(&(bmp->header.second_header.iYpelsPerMeter), sizeof(int), 1, fd);	// 42 - 45
	fwrite(&(bmp->header.second_header.iClrUsed), sizeof(int), 1, fd);			// 46 - 49
	fwrite(&(bmp->header.second_header.iClrImportant), sizeof(int), 1, fd);		// 50 - 53

	for (i = 0; i < bmp->height; i++)
	{
		for (j = 0; j < bmp->width; j++)
		{
			fputc(bmp->data[i][j].b, fd);
			fputc(bmp->data[i][j].g, fd);
			fputc(bmp->data[i][j].r, fd);
		}
		for (j = 0; j < bmp->width_useless; j++)
			fputc(0, fd);
	}
	fclose(fd);
	return (1);
}

/// <summary>
/// Repopulates the image file given a 1D array
/// </summary>
void createImage(t_bmp *img, unsigned char *anArray)
{
    int i, j, k;
	int width = img->width;

	// Must reverse height
    k = (img->height * img->width) - 1;
    for(i = 0; i < img->height; i++)
    {
        for (j = 0; j < width; j++)
        {
			// create a white pixel
            if (anArray[k] == 1)
	        {
		        img->data[i][j].r = (unsigned char)255;
		        img->data[i][j].g = (unsigned char)255;
		        img->data[i][j].b = (unsigned char)255;
		        img->data[i][j].moy = (unsigned char)255;
	        }
			// create a red pixel
			else if (anArray[k] == 2)
			{
		        img->data[i][j].r = (unsigned char)255;
		        img->data[i][j].g = (unsigned char)0;
		        img->data[i][j].b = (unsigned char)0;
		        img->data[i][j].moy = (unsigned char)255;
			}
			// create a green pixel
			else if (anArray[k] == 3)
			{
		        img->data[i][j].r = (unsigned char)0;
		        img->data[i][j].g = (unsigned char)255;
		        img->data[i][j].b = (unsigned char)0;
		        img->data[i][j].moy = (unsigned char)255;
			}
			// create a blue pixel
			else if (anArray[k] == 4)
			{
				img->data[i][j].r = (unsigned char)0;
		        img->data[i][j].g = (unsigned char)0;
		        img->data[i][j].b = (unsigned char)255;
		        img->data[i][j].moy = (unsigned char)255;
			}
			// create a black pixel
			else if (anArray[k] == 0)
			{
				img->data[i][j].r = (unsigned char)0;
		        img->data[i][j].g = (unsigned char)0;
		        img->data[i][j].b = (unsigned char)0;
		        img->data[i][j].moy = (unsigned char)0;
			}
			// create a turquoise pixel
	        else
	        {
		        img->data[i][j].r = (unsigned char)0;
		        img->data[i][j].g = (unsigned char)255;
		        img->data[i][j].b = (unsigned char)255;
		        img->data[i][j].moy = (unsigned char)255;
	        }
			k--;
        }
    }
}

/// <summary>
/// Used to place an image into a 1D array for processing
/// </summary>
void imageToArray(t_bmp *img, unsigned char *anArray)
{
	int i, j, k;

	// reverse height and place into array
	k = (img->height * img->width) - 1;
	for (i = 0; i < img->height; i++)
	{
		for (j = 0; j < img->width; j++)
		{
			//white is a 1
			if (img->data[i][j].moy == 255)
			{
 				anArray[k] = 1;
			}
			// If it is blue.
			else if (img->data[i][j].moy == (unsigned char) 85)
			{
 				anArray[k] = 4;
			}
			// Else all pixel are set to black
			else
			{
				anArray[k] = 0;
			}
			k--;
		}
	}
}