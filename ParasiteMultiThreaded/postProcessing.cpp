#include "parasite.h"

/*
 * This file contains all methods related to image post-processing
 */

/// <summary>
/// Dilates a given image which is represented by the linear array 'anArray'
/// </summary>
void dilateImage(int height, int width, unsigned char *anArray)
{
    unsigned char *tempArray = (unsigned char*)malloc(sizeof(unsigned char) * height * width);
    int i, j;

    for (i = 0; i < width * height; i++)
    {
        tempArray[i] = anArray[i];
    }

	//// Unoptimised version - iterates through all black pixels checking their neighbourhood
	//// Perform logical or operation to dilate image
	//for (i = 1; i < height - 1; i++)
	//{
	//    for (j = 1; j < width - 1; j++)
	//    {
	//		if (anArray[(i * width) + j] == 0)
	//		{
	//			if (anArray[((i - 1) * width) + (j - 1)] == 1 || anArray[((i - 1)* width) + j]  == 1 || 
	//				anArray[((i - 1) * width) + (j + 1)] == 1 || anArray[(i * width) + (j - 1)] == 1 ||
	//				anArray[(i * width) + (j + 1)] == 1 || anArray[((i + 1) * width) + (j - 1)] == 1 ||
	//				anArray[((i + 1) * width) + j] == 1 || anArray[((i + 1) * width) + (j + 1)] == 1)
	//				{
	//					tempArray[(i * width) + j] = 1;
	//				}
	//		}
	//    }
 	//}

	//Optimised version - only parses white pixels checking their neighbourhood
	for (i = 1; i < height - 1; i++)
    {
        for (j = 1; j < width - 1; j++)
        {
			if (anArray[(i * width) + j] == 1)
			{
				tempArray[((i - 1) * width) + (j - 1)] = 1;
				tempArray[((i - 1)* width) + j]  = 1;
				tempArray[((i - 1) * width) + (j + 1)] = 1;
				tempArray[(i * width) + (j - 1)] = 1;
				tempArray[(i * width) + (j + 1)] = 1;
				tempArray[((i + 1) * width) + (j - 1)] = 1;
				tempArray[((i + 1) * width) + j] = 1;
				tempArray[((i + 1) * width) + (j + 1)] = 1;

				tempArray[(i * width) + j] = 1;
			}
        }
    }
	
    for (i = 0; i < width * height; i++)
    {
        anArray[i] = tempArray[i];
    }

	free(tempArray);
}

/// <summary>
/// Erodes a given image which is represented by the linear array 'anArray'
/// </summary>
void erodeImage(int height, int width, unsigned char *anArray)
{
	unsigned char *tempArray = (unsigned char*)malloc(sizeof(unsigned char) * height * width);
    int i, j;

	for (i = 0; i < width * height; i++)
    {
        tempArray[i] = anArray[i];
    }

	// Perform logical and operation to erode image
    for (i = 1; i < height - 1; i++)
    {
        for (j = 1; j < width - 1; j++)
        {
			if (anArray[(i * width) + j] == 1)
			{
				if (anArray[((i - 1) * width) + (j - 1)] != 1 || anArray[((i - 1)* width) + j]  != 1 || 
					anArray[((i - 1) * width) + (j + 1)] != 1 || anArray[(i * width) + (j - 1)] != 1 ||
					anArray[(i * width) + (j + 1)] != 1 || anArray[((i + 1) * width) + (j - 1)] != 1 ||
					anArray[((i + 1) * width) + j] != 1 || anArray[((i + 1) * width) + (j + 1)] != 1)
					{
						tempArray[(i * width) + j] = 0;
					}
			}
        }
    }

	for (i = 0; i < width * height; i++)
    {
        anArray[i] = tempArray[i];
    }

	free(tempArray);
}

/// <summary>
/// This method is designed to remove remaining noise in the vertial direction after post-processing
/// Only removes groups of pixel below the defined mask size
/// </summary>
void removeUnwantedNoiseHeight(int height, int width, unsigned char *anArray, int maskSize)
{
	unsigned char* tempArray = (unsigned char*)malloc(sizeof(unsigned char) * height * width);
	int unwantedHeight = maskSize;
	int i, j, heightCount;

	for (i = 0; i < width * height; i++)
    {
        tempArray[i] = anArray[i];
    }

	// search until a pixel value of 1 is found
	// find size of object
	// if the size of the object is less than 'maskSize' then remove
	for (i = 2; i < height - 2; i++)
    {
        for (j = 0; j < width; j++)
        {
			heightCount = 0;

			//check pixels
			if (anArray[(i * width) + j] == 1)
			{
				heightCount++;

				//check below 'unwantedHeight' pixels
				for (int k = 1; k < unwantedHeight; k++)
				{
					if (anArray[((i + k) * width) + j] == 1)
						heightCount++;
				}
				//check above 'unwantedHeight' pixels
				for (int k = 1; k < unwantedHeight; k++)
				{
					// Needed to include this check to stop array out of bounds if the maskSize was too big.
					if(!((i-k) < 0))
						if (anArray[((i - k) * width) + j] == 1)
							heightCount++;
				}
			}
			//Check pixel height and delete if necessary
			if(heightCount < unwantedHeight)
			{
				tempArray[(i * width) + j] = 0;
			}
		}
	}

	for (i = 0; i < width * height; i++)
    {
        anArray[i] = tempArray[i];
    }

	free(tempArray);

}

/// <summary>
/// This method is designed to remove remaining noise in the horizontal direction after post-processing
/// Only removes groups of pixel below the defined mask size
/// </summary>
void removeUnwantedNoiseWidth(int height, int width, unsigned char *anArray, int maskSize)
{
	unsigned char* tempArray = (unsigned char*)malloc(sizeof(unsigned char) * height * width);
	int unwantedWidth = maskSize;
	int i, j, widthCount;

	for (i = 0; i < width * height; i++)
    {
        tempArray[i] = anArray[i];
    }

	// search until a pixel value of 1 is found
	// find size of object
	// if the size of the object is less than 'maskSize' then remove
	for (i = 0; i < height; i++)
    {
        for (j = 2; j < width - 2; j++)
        {
			widthCount = 0;

			//check pixels
			if (anArray[(i * width) + j] == 1)
			{
				widthCount++;
				
				//check right 'unwantedWidth' pixels
				for (int k = 1; k < unwantedWidth; k++)
				{
					if (anArray[(i * width) + (j+k)] == 1)
						widthCount++;
				}
				//check left 'unwantedWidth' pixels
				for (int k = 1; k < unwantedWidth; k++)
				{
					if (anArray[(i * width) + (j-k)] == 1)
						widthCount++;
				}
			}
			//Check pixel width and delete if necessary
			if(widthCount < unwantedWidth)
			{
				tempArray[(i * width) + j] = 0;
			}
		}
	}

	for (i = 0; i < width * height; i++)
    {
        anArray[i] = tempArray[i];
    }

	free(tempArray);
}