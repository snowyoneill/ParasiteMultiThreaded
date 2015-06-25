#include "parasite.h"

/*
 * This file contains all methods related to image pre-processing and segmentation
 */

/// <summary>
/// Create a histogram of the image
/// </summary>
short* createHistogram(t_bmp *img)
{
	short *moyLevel = (short*)malloc(sizeof(short) * 256);

	int i, j;

	// Initialise moy array
	for (i = 0; i < 256; i++)
	{
		moyLevel[i] = 0;
	}

	// Get total number of pixels for each moy level
	for (i = 0; i < img->height; i++)
	{
		for (j = 0; j < img->width; j++)
		{
			moyLevel[img->data[i][j].moy]++;
		}
	}

	return moyLevel;
}

/// <summary>
/// Carries out contrast enhancement on the given image.
/// </summary>
void enhanceContrast(t_bmp *img, short *histogram)
{
	int i, j, outputValue;
	float startPoint, endPoint, gradient, intercept;
	startPoint = -1;
	endPoint = -1;

	i = 0;
	while(startPoint == -1)
	{
		if (histogram[i] != 0)
		{
			startPoint = (float) i;
		}
		i++;
	}

	i = 255;
	while(endPoint == -1)
	{
		if (histogram[i] != 0)
		{
			endPoint = (float) i;
		}
		i--;
	}
	
	//printf("start of histogram: %f \n", startPoint);
	//printf("end of histogram: %f \n", endPoint);

	// Get gradient and y axis intercept
	gradient = 255 / (endPoint - startPoint);
	intercept = -gradient * startPoint;

	// Modify the image using new values O = gradient * input + intercept
	// Contract enhancement using linear stretching
	for(i = 0; i < img->height; i++)
	{
		for(j = 0; j < img->width; j++)
		{
			outputValue = (int) ((gradient * img->data[i][j].moy) + intercept);
			if (img->data[i][j].moy < -intercept/gradient)
			{
				img->data[i][j].moy = 0;
				img->data[i][j].r = 0;
				img->data[i][j].g = 0;
				img->data[i][j].b = 0;
			}
			else if (img->data[i][j].moy > (255 - intercept)/gradient)
			{
				img->data[i][j].moy = 255;
				img->data[i][j].r = 255;
				img->data[i][j].g = 255;
				img->data[i][j].b = 255;
			}
			else
			{
				img->data[i][j].moy = outputValue;
				img->data[i][j].r = outputValue;
				img->data[i][j].g = outputValue;
				img->data[i][j].b = outputValue;
			}
		}
	}

}

/// <summary>
/// Carries out a low pass filter on the given image.
/// </summary>
void lowPassFilterImage(t_bmp *img, t_bmp *outputImg)
{
	// Mask size of 9.
	short filter[9];

	int count;

	for (int i = 0; i < img->height; i++)
	{
		for (int j = 0; j < img->width; j++)
		{
			count = 0;
			memset(filter, 0, sizeof(short)*9);

			if (i != 479)				 { filter[0] = img->data[i+1][j].moy; count++; }	// Pixel above
			if (i != 0)					 { filter[1] = img->data[i-1][j].moy; count++; }	// Pixel below
			if (j != 0)					 { filter[2] = img->data[i][j-1].moy; count++; }	// Left pixel
			if (j != 639)				 { filter[3] = img->data[i][j+1].moy; count++; }	// Right pixel

			if ((i != 0) && (j != 0))	 { filter[5] = img->data[i-1][j-1].moy; count++; }	// Bottom left
			if ((i != 0) && (j != 639))	 { filter[6] = img->data[i-1][j+1].moy; count++; }	// Bottom pixel		
			if ((i != 479) && (j != 0))	 { filter[7] = img->data[i+1][j-1].moy;	count++; }	// Top Left
			if ((i != 479) && (j != 639)){ filter[8] = img->data[i+1][j+1].moy; count++; }	// Top Right

			filter[4] = img->data[i][j].moy; count++;		// Middle

			short lowPassValue = (filter[0] + filter[1] + filter[2] + filter[3] + filter[4] + filter[5] + filter[6] + filter[7] + filter[8])/count;

			outputImg->data[i][j].moy = (unsigned char)lowPassValue;
			outputImg->data[i][j].r = (unsigned char)lowPassValue;
			outputImg->data[i][j].g = (unsigned char)lowPassValue;
			outputImg->data[i][j].b = (unsigned char)lowPassValue;
		}
	}
}

/// <summary>
/// Carries out a low pass filter on the given image.
/// </summary>
//void lowPassFilterImage(t_bmp *img)  //This implementation is wrong because it writes the result to the same buffer/image instead of a new one.
//{
//	// Mask size of 9.
//	short filter[9];
//
//	for (int i = 1; i < (img->height - 1); i++)
//	{
//		for (int j = 1; j < (img->width - 1); j++)
//		{
//			filter[0] = img->data[i-1][j-1].moy;
//			filter[1] = img->data[i-1][j].moy;
//			filter[2] = img->data[i-1][j+1].moy;
//			//filter[3] = img->data[i][j-1].moy;
//
//			filter[3] = img->data[1][1].moy;
//			filter[4] = img->data[i][j].moy;
//			filter[5] = img->data[i][j+1].moy;
//			filter[6] = img->data[i+1][j-1].moy;
//			filter[7] = img->data[i+1][j].moy;
//			filter[8] = img->data[i+1][j+1].moy;
//
//			short lowPassValue = 0;
//			lowPassValue = (filter[0] + filter[1] + filter[2] + filter[3] + filter[4]
//								  + filter[5] + filter[6] + filter[7] + filter[8])/9;
//
//				img->data[i][j].moy = (unsigned char)lowPassValue;
//				img->data[i][j].r = (unsigned char)lowPassValue;
//				img->data[i][j].g = (unsigned char)lowPassValue;
//				img->data[i][j].b = (unsigned char)lowPassValue;
//		}
//	}
//}

/// <summary>
/// Median Filter the image based upon moy values
/// </summary>
void medianFilterImage(t_bmp *img, t_bmp *outputImg)
{
	int i, j;
	short filter[9];

	for (i = 1; i < (img->height - 1); i++)
	{
		for (j = 1; j < (img->width - 1); j++)
		{ 
			filter[0] = img->data[i-1][j-1].moy;
			filter[1] = img->data[i-1][j].moy;
			filter[2] = img->data[i-1][j+1].moy;
			filter[3] = img->data[i][j-1].moy;
			filter[4] = img->data[i][j].moy;
			filter[5] = img->data[i][j+1].moy;
			filter[6] = img->data[i+1][j-1].moy;
			filter[7] = img->data[i+1][j].moy;
			filter[8] = img->data[i+1][j+1].moy;

			sortArray(filter, 9);

			outputImg->data[i][j].moy = (unsigned char)filter[4];
			outputImg->data[i][j].r = (unsigned char)filter[4];
			outputImg->data[i][j].g = (unsigned char)filter[4];
			outputImg->data[i][j].b = (unsigned char)filter[4];
		}
	}
}

/// <summary>
/// Median Filter the image based upon moy values
/// </summary>
//void medianFilterImage(t_bmp *img)  //This implementation is wrong because it writes the result to the same buffer/image instead of a new one.
//{
//	int i, j;
//	//int debugX;
//	short filter[9];
//
//	for (i = 1; i < (img->height - 1); i++)
//	{
//		for (j = 1; j < (img->width - 1); j++)
//		{ 
//			filter[0] = img->data[i-1][j-1].moy;
//			filter[1] = img->data[i-1][j].moy;
//			filter[2] = img->data[i-1][j+1].moy;
//			filter[3] = img->data[i][j-1].moy;
//			filter[4] = img->data[i][j].moy;
//			filter[5] = img->data[i][j+1].moy;
//			filter[6] = img->data[i+1][j-1].moy;
//			filter[7] = img->data[i+1][j].moy;
//			filter[8] = img->data[i+1][j+1].moy;
//
//			////////debug
//			/*
//			printf("debug info unsorted \n");
//			for(debugX = 0; debugX<9; debugX++)
//			{
//
//				printf("value %d is %d \n", debugX, filter[debugX]);
//			}*/
//			/////////////
//
//			sortArray(filter, 9);
//
//			////////debug
//			/*
//			printf("debug info sorted\n");
//			for(debugX = 0; debugX<9; debugX++)
//			{
//
//				printf("value %d is %d \n", debugX, filter[debugX]);
//			}*/
//			/////////////
//
//			img->data[i][j].moy = (unsigned char)filter[4];
//			img->data[i][j].r = (unsigned char)filter[4];
//			img->data[i][j].g = (unsigned char)filter[4];
//			img->data[i][j].b = (unsigned char)filter[4];
//
//		}
//	}
//}

/// <summary>
/// Calculate an automatic threshold based on the ISODATA approach
/// </summary>
int iterativeCalculateThreshold(t_bmp *img)
{
	int i, j, thresholdValue, newThresholdValue, objectSetSize, backgroundSetSize, objectSetValue, backgroundSetValue,
		objectSetAverage, backgroundSetAverage, thresholdFound;
	
	thresholdValue = -1;
	// newThresholdValue = 128;  //Half the dynamic range
	newThresholdValue = 255;  //Max dynamic range -1
	thresholdFound = 0;

	while(thresholdFound != 1)
	{
		thresholdValue = newThresholdValue;

		objectSetSize = 0;
		backgroundSetSize = 0;
		objectSetValue = 0;
		backgroundSetValue = 0;

		for (i = 0; i < img->height; i++)
		{
			// Get size and value of background and object sets
			for (j = 0; j < img->width; j++)
			{
				if (img->data[i][j].moy > thresholdValue)
				{
					objectSetSize++;
					objectSetValue += img->data[i][j].moy;
				}
				else
				{
					backgroundSetSize++;
					backgroundSetValue += img->data[i][j].moy;
				}
			}
		}

		// if the foreground object size is 0 simply set it to 1
		objectSetSize = (objectSetSize == 0) ? 1 : objectSetSize;
		// Get average of background and object sets
		objectSetAverage = objectSetValue / objectSetSize;
		backgroundSetAverage = backgroundSetValue / backgroundSetSize;

		newThresholdValue = (objectSetAverage + backgroundSetAverage) / 2;

		// if the current threshold value equals the old then the appropriate threshold has been found.
		if (thresholdValue == newThresholdValue)
			thresholdFound = 1;
	}

	return newThresholdValue;
}

/// <summary>
/// Segments the given image using the automatically calculated threshold.
/// </summary>
void segmentImage(t_bmp *img)
{
	int i, j;
	unsigned char threshold = iterativeCalculateThreshold(img);
	//unsigned char threshold = 100;
	threshold = (threshold > 140) ? 125 : threshold;

	for (i = 0; i < img->height; i++)
	{
		for (j = 0; j < img->width; j++)
		{
			// sets pixel to black if the current moy is greater than the threshold
			if (img->data[i][j].moy > threshold)
			{
				img->data[i][j].r = (unsigned char)0;
				img->data[i][j].g = (unsigned char)0;
				img->data[i][j].b = (unsigned char)0;
				img->data[i][j].moy = (unsigned char)0;
			}
			// sets pixel to white
			else
			{
				img->data[i][j].r = (unsigned char)255;
				img->data[i][j].g = (unsigned char)255;
				img->data[i][j].b = (unsigned char)255;
				img->data[i][j].moy = (unsigned char)255;
			}
		}
	}
}