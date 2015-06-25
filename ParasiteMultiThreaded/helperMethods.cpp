#include "parasite.h"

/*
 * This file contains extra methods which are utilised by various functions to carry out mainly I/O operations
 */

/// <summary>
/// Calculates the number of images that are contained within a given directory by reading the integer appended to
/// the 'numberOfFrame.txt' file.
/// </summary>
int getNumberOfImages(char* directory)
{
	int ret;
	FILE *fr;
	
	// Allocates a 255 chars to store the length of the directory path.
	char* numberOfFramesFile = (char*)malloc(sizeof(char)*255);
	strcpy(numberOfFramesFile, (char*) directory);
	strcat(numberOfFramesFile, "\\numberOfFrames.txt");
	fr = fopen(numberOfFramesFile, "r");
	// if the file handle is NULL display an error message
	if(fr == NULL) 
	{
		printf("Cannot read the numberOfImages.txt input file.\n");
		getchar();
		exit(1);
	}
	// free the path
	free(numberOfFramesFile);
	// read the number contained in the text file
    fscanf(fr, "%d", &ret);
	// close the stream
	fclose(fr);
	//return the number
	return ret;
}

/// <summary>
/// Sorts a list of shorts in accending order
/// </summary>
void sortArray(short *anArray, int arrayLength) 
{
  int i, j, key;

  for (j = 1; j < arrayLength; j++) 
  {
    key = anArray[j];
    i = j - 1;
    while ((i >= 0) && (key < anArray[i])) 
	{
      anArray[i + 1] = anArray[i];
      i--;
    }
    anArray[i + 1] = key;
  }
}

/// <summary>
/// Sorts a list of doubles in accending order
/// </summary>
void selectSort(double array[], int n)
{
      for(int i = 0; i<n; i++)
      {
         int min = i;
		 double minVal = array[i];
         for (int j = i; j <n; j++)
			 if (array[j] < minVal)
			 {
				 min = j;
				 minVal = array[j];
			 }
         array[min] = array[i];
         array[i] = minVal;
      } 
}

/// <summary>
/// Search an array for largest value
/// </summary>
short largestSearch(short* anArray, int arraySize)
{
	int i;
	short largestValue = anArray[0];
	for(i = 1; i < arraySize; i++)
	{
		if(anArray[i] > largestValue)
		{
			largestValue = anArray[i];
		}
	}
	return largestValue;
}

/// <summary>
/// Search an array for smallest value
/// </summary>
short smallestSearch(short* anArray, int arraySize)
{
	int i;
	short smallestValue = anArray[0];
	for(i = 1; i < arraySize; i++)
	{
		if(smallestValue > anArray[i])
		{
			smallestValue = anArray[i];
		}
	}
	return smallestValue;
}

/// <summary>
/// Calculate the mean of an image's gray values
/// </summary>
float calculateMean(t_bmp *img)
{
	int i, j;
	float mean = 0;

	for (i = 0; i < img->height; i++)
	{
		for (j = 0; j < img->width; j++)
		{
			mean += img->data[i][j].moy;
		}
	}

	mean /= img->height * img->width;

	return mean;
}

/// <summary>
/// Calculate the standard deviation of an image's gray values
/// </summary>
float calculateStandardDeviation(t_bmp *img, float mean)
{
	int i, j;
	float standardDeviation = 0;

	for (i = 0; i < img->height; i++)
	{
		for (j = 0; j < img->width; j++)
		{
			standardDeviation += ((mean - img->data[i][j].moy) * (mean - img->data[i][j].moy));
		}
	}
	standardDeviation /= (float)(img->height * img->width);
	standardDeviation = (float) sqrt(standardDeviation);

	return standardDeviation;
}

/// <summary>
/// Compensate for the mid point finding method being inadequate
/// </summary>
void compensateForSmallSize(short *anArray, int length)
{
	int i, j, startPoint, endPoint;

	for (i = 1; i < length; i++)
	{
		if (anArray[i] <= 10) 
		{
			j = i+1;
			startPoint = anArray[i-1];
			endPoint = -1;
			while (endPoint == -1)
			{
				if(j > length-1)
				{
					endPoint = startPoint;
				}
				else if(anArray[j] > 10)
				{
					endPoint = anArray[j];
				}
				else
				{
					j++;
				}
			}
			anArray[i] = (startPoint + endPoint) / 2;
		}
	}
}

/// <summary>
/// Copies the contents of the original array to the duplicate array
/// </summary>
void copyArray(unsigned char *orgArray, unsigned char *duplicateArray)
{
	for(int i = 0; i < (MAX_DATA_SIZE); i++)
		duplicateArray[i] = orgArray[i];
}

/// <summary>
/// Outputs 2 arrays 'array1' and 'array2' to the filenames specified 'arrayName1' and 'arrayName2'
/// </summary>
void outputArrays(unsigned char *array1, char *arrayName1, unsigned char *array2, char *arrayName2)
{
	//This section writes both arrays to text files which can then be checked to ensure they are identical.

	char* name1 = (char*)malloc(sizeof(char)*10);
	strcpy(name1, arrayName1);
	strcat(name1, ".txt");
	char* name2 = (char*)malloc(sizeof(char)*10);
	strcpy(name2, arrayName2);
	strcat(name2, ".txt");

	FILE *per_out_file;
	FILE *an_out_file;
	per_out_file = fopen(name1, "w");
	an_out_file = fopen(name2, "w");

	for(int i = 0; i < (MAX_DATA_SIZE); i++){
		//fprintf(per_out_file, "array1[%d] %d.\n", i , array1[i]);
		fprintf(per_out_file, "array[%d] %d.\n", i , array1[i]);
	}

	for(int i = 0; i < (MAX_DATA_SIZE); i++){
		//fprintf(an_out_file, "array2[%d] %d.\n", i, array2[i]);
		fprintf(an_out_file, "array[%d] %d.\n", i, array2[i]);
	}

	free(name1);
	free(name2);

	fclose(per_out_file);
	fclose(an_out_file);
}

/// <summary>
/// Outputs a single array 'array1' to the filename specified
/// </summary>
void outputArray(unsigned char *array1, char *arrayName)
{
	char* name = (char*)malloc(sizeof(char)*10);
	strcpy(name, arrayName);
	strcat(name, ".txt");

	FILE *per_out_file;
	per_out_file = fopen(name, "w");

	for(int i = 0; i < (MAX_DATA_SIZE); i++){
		fprintf(per_out_file, "array[%d] %d.\n", i , array1[i]);
	}

	free(name);
	fclose(per_out_file);
}