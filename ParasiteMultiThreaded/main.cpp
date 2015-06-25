#include "parasite.h"

//#include <conio.h>;
//#include <iostream>;

////////////////////////////////////////////////////////////////////////////////
/* Custom HPC Counter                                                         */
////////////////////////////////////////////////////////////////////////////////

//#define BENCHMARKING  // Cant benchmark multiple threads - timer is not an instance - it is static
//#define ELAPSED
//#define NUM_OF_PASSES 1
//#define DEBUG_OUTPUT

extern "C" {
   #include "hr_time.h"
}

extern "C" {
   #include "cputime.h"
}

//extern "C" {
//   #include "leak_detector_c.h"
//}

////////////////////////////////////////////////////////////////////////////////
/* End Custom HPC Counter                                                     */
////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
///* Timeb headers                                                              */
//////////////////////////////////////////////////////////////////////////////////
//#include <sys/timeb.h>
//#include <time.h>

//// to get portable code we need this
//#ifdef WIN32 // _WIN32 is defined by all Windows 32 compilers, but not by others.
//#define timeb _timeb
//#define ftime _ftime
//#endif
//////////////////////////////////////////////////////////////////////////////////
///* End timeb headers                                                          */
//////////////////////////////////////////////////////////////////////////////////

//#include "mmgr.h"


// Global variables
#ifdef CAL_SIZE
	short* parasiteSize;
#endif
double* parasiteCompactness;
int noOfThreads;
int imagesPerThread;
char* imagePath;

// declare an array which will store a range of compactness values
double compactnessThreshold;
// prototype function definition
void processImages(struct threadStats*);

int main(int argc, char* argv[])
{
	// Define a collection of handles and thread stats structure
	HANDLE *threads;
	struct threadStats *st;
	int numberOfInputImages, extra;

	// get the index of the image we want and make a unsigned char array of it then concat it to imageFile.
	// Set the size of the variable 'imageFileName' to 255 characters
	imagePath = (char*)malloc(sizeof(char)*255);
	memset(imagePath, 0, (sizeof(char)*255));

	int imageIndex = 1;
	//// Construct the file name
	//char imageIndexString[10];
	//itoa(imageIndex,imageIndexString,10);

	//strcpy(imagePath, "images/colour");
	//strcat(imagePath, imageIndexString);
	//strcat(imagePath, ".bmp");

	// Set the size of the video name variable 30 characters.
	char* videoName = (char*)malloc(sizeof(char)*30);
	if (argc != 4) /* argc should be 4 for correct execution */
	{
	    printf("No folder or video name specified or invalid number of arguments.\n");
		getchar();
		return 1;
	}
	else
	{
		//printf("path: %s\n", argv[1]);
		// copy the first argument to the 'imagePath' variable.
		strcpy(imagePath, (char*) argv[1]);
		#ifndef RELEASE_BUILD
			strcat(imagePath,"\images");
		#endif
		// copy the second argument to the 'videoName' variable.
		strcpy(videoName, (char*) argv[2]);
		//extract the compactness threshold from the list of command line args
		compactnessThreshold = atof((argv[3]));
	}
	//printf("imagePath: %s\n", imagePath);
	//printf("compactnessThreshold: %d\n", compactnessThreshold);
	
	// Set the integer 'numberOfInputImages'.
	numberOfInputImages = getNumberOfImages(imagePath);
	//numberOfInputImages = 1;
	//numberOfInputImages = 140;

	// Detect the total number of available cores
	int numCores = 1;
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	numCores = siSysInfo.dwNumberOfProcessors;

	printf("////////////////////////////////////////\n");
	printf("/    MULTITHREADED PARASITE ANALYSIS   /\n");
	printf("////////////////////////////////////////\n");
	printf("Starting program...\n\n");
	printf("Image Path: %s\n", imagePath);
	printf("Compactness Threshold Selected: %f\n", compactnessThreshold);
	printf("#Input Images: %d\n", numberOfInputImages);
	printf("%d Cores available.\n", numCores);
	printf("___________________\n\n");

	#ifdef CAL_SIZE
		parasiteSize = (short *)malloc(sizeof(short) * numberOfInputImages);
	#endif
	parasiteCompactness = (double *)malloc(sizeof(double) * numberOfInputImages);
	
	// setup user input
	#ifdef USER_INPUT
		printf("Enter the number of threads: ");
		scanf("%d",&noOfThreads);
		
		// cannot create more threads than cores available
		while(noOfThreads>numCores)
		{
			printf("There is only %d cores available - therefore cannot create more than %d thread(s)\n", numCores, numCores);
			printf("\n");
			printf("Enter the number of threads: ");
			scanf("%d",&noOfThreads);
		}

		// cannot create more threads than images
		while(noOfThreads>numberOfInputImages)
		{
			printf("There is only %d images - therefore cannot create more than %d thread(s)\n", numberOfInputImages, numberOfInputImages);
			printf("\n");
			printf("Enter the number of threads: ");
			scanf("%d",&noOfThreads);
		}
	#else
		noOfThreads = numCores;
		if (noOfThreads > numberOfInputImages)
			noOfThreads = numberOfInputImages;
		printf("Choosing optimal number of threads...\n");
	#endif

	// calculate the number of images each thread should process
	imagesPerThread = numberOfInputImages/noOfThreads;
	// calculate the amount of leftover images
	extra = numberOfInputImages % noOfThreads;

	//// declare a number of slave threads - used when the master thread did some processing
	//threads	= (HANDLE*) malloc(sizeof(HANDLE)*(noOfThreads-1));				// 1 handle for each spawned thread not counting master thread
	threads	= (HANDLE*) malloc(sizeof(HANDLE)*(noOfThreads));
	st = (struct threadStats*) malloc(sizeof(struct threadStats)*noOfThreads);	// struct for each thread

	double processElapsedTime = 0;
	//int offset = imagesPerThread;
	int offset = 0;
	// declare a high resolution timer for benchmarking
	stopWatch multi_stopwatch;

	#ifdef RUNS
		FILE *runOutput;
		runOutput = fopen("Run_Results.txt", "w");
		fprintf(runOutput, "Pass\tTime\n");

		double totalRuntime = 0;
	#endif

	if(initialiseTimer(&multi_stopwatch)==0)
	{
		#ifdef RUNS
		for(int k=0; k<NUM_RUNS; k++)
		{
			printf("Run number: %d\n", k+1);
		#endif

			startTimer(&multi_stopwatch, 1);

			//int noOfSlaves = noOfThreads-1;
			//for(int i=1;i<=noOfSlaves;i++)
			for(int i=0;i<noOfThreads;i++)
			{
				// Setup each threads ID, workload and offset
				st[i].id = i;
				st[i].imagesPerThread = imagesPerThread;
				if (extra > 0) {
					st[i].imagesPerThread += 1;
					extra--;
				}
				st[i].imageOffset = offset;
				offset += st[i].imagesPerThread;
				
				//// Spawn new threads to process images
				//threads[i] = (HANDLE) _beginthreadex(NULL,0,processImages,(void*)&st[i],0,NULL);
				//threads[i-1] = (HANDLE) _beginthread((void(*)(void*))processImages, 0, (void*)&st[i]);
				threads[i] = (HANDLE) _beginthread((void(*)(void*))processImages, 0, (void*)&st[i]);
			}

			//st[0].id = 0;
			//st[0].imagesPerThread = imagesPerThread;
			//st[0].imageOffset = 0;
			//processImages(&st[0]);

			//// Wait for all threads to terminate
			//WaitForMultipleObjects(noOfSlaves, threads, TRUE, INFINITE);
			WaitForMultipleObjects(noOfThreads, threads, TRUE, INFINITE);
			
			stopTimer(&multi_stopwatch, 1);

			#ifndef RUNS
				processElapsedTime += getElapsedTimeInMilli(&multi_stopwatch);
			#endif

		#ifdef RUNS
		totalRuntime += getElapsedTime(&multi_stopwatch);
		fprintf(runOutput, "%d\t%f\n", k, getElapsedTime(&multi_stopwatch));
		offset = 0;
		extra = numberOfInputImages % noOfThreads;
		}
		#endif

		printf("\n");
		printf("Free thread handles and structs\n");
		free(st);
		free(threads);

		printf("All threads have completed.\n\n");
	}
	else { printf("System timer did not initialise\n"); }

	#ifdef RUNS
		#ifdef USER_INPUT
			getchar(); // last character in stream will be a return
		#endif
		printf("Run results appended to file.\n");
		fprintf(runOutput, "Average:\t%f\n", totalRuntime/NUM_RUNS);
		fclose(runOutput);
	#else

		printf("# Cores: %d\n", siSysInfo.dwNumberOfProcessors);
		printf("# Images: %d\n", numberOfInputImages);
		printf("# Threads: %d\n", noOfThreads);
		printf("# Images per thread: %d\n", imagesPerThread);

		//printf("Size of Struct: %d\n", sizeof(struct threadStats)*noOfThreads);
		printf("\nTotal Elapsed Time: %f(s), %f(ms)\n\n", getElapsedTime(&multi_stopwatch), processElapsedTime);
		printf("\n");

		#ifdef USER_INPUT
			getchar(); // used to catch return key from text input
		#endif
		fflush(NULL);
		//printf("Please type enter to obtain results\n");
		//getchar();
		////system("pause");

		// Create a text file to store the results
		#ifdef TXT
			FILE *txtOutput;
			//txtOutput = fopen("Results.txt", "w");
			txtOutput = fopen("Results.txt", "a"); // append to the current results file
			fprintf(txtOutput, "%s\n", videoName);
			fprintf(txtOutput, "Compactness Threshold: %f\n", compactnessThreshold);
			fprintf(txtOutput, "Total Elapsed Time(s):\t%f\n", getElapsedTime(&multi_stopwatch));
			#ifdef CAL_SIZE
				fprintf(txtOutput, "Image No\tSize\tCompactness\n");
			#else
				fprintf(txtOutput, "Image No\tCompactness\n");
			#endif
		#endif

		// Create a csv file to store the results
		#ifdef CSV
			FILE *csvOutput;
			csvOutput = fopen("Results.csv", "a"); // append to the current results file
			fprintf(csvOutput, "%s\n", videoName);
			fprintf(csvOutput, "Compactness Threshold: %f\n", compactnessThreshold);
			fprintf(csvOutput, "Total Elapsed Time(s):,%f\n", getElapsedTime(&multi_stopwatch));
			#ifdef CAL_SIZE
				fprintf(csvOutput, "Image No,Size,Compactness\n");
			#else
				fprintf(csvOutput, "Image No,Compactness\n");
			#endif
		#endif

		//Compensate for worm sizes being too small
		#ifdef CAL_SIZE
			compensateForSmallSize(parasiteSize, numberOfInputImages);
		#endif

		// Output the compactness values
		#ifndef TXT
			printf("--------------- Flux Unsorted ---------------\n");
		#endif
		for(int i = 0; i < numberOfInputImages; i++)
		{
			#ifndef TXT
			#ifndef CSV
				if (i%imagesPerThread == 0)
					printf("++++++++++++++++\n");
				printf("Array index %d: size = %d \t compactness = %f\n" , i, parasiteSize[i], parasiteCompactness[i]);
			#endif
			#endif

			#ifdef TXT
				#ifdef CAL_SIZE
					fprintf(txtOutput, "%d\t%d\t%f\n" , i, parasiteSize[i], parasiteCompactness[i]);
				#else
					fprintf(txtOutput, "%d\t%f\n" , i, parasiteCompactness[i]);
				#endif
			#endif

			#ifdef CSV
				#ifdef CAL_SIZE
					fprintf(csvOutput, "%d,%d,%f\n" , i, parasiteSize[i], parasiteCompactness[i]);
				#else
					fprintf(csvOutput, "%d,%f\n" , i, parasiteCompactness[i]);
				#endif
			#endif
		}

		#ifdef TXT
			fprintf(txtOutput, "\n");
		#endif

		#ifdef CSV
			fprintf(csvOutput, "\n");
		#endif

		// Calculate the number of worm fluctuations and write the output to the disk
		int flux;
		if (numberOfInputImages >= 3) {
			#ifdef TXT
				flux = findParasiteFluxAlt(parasiteCompactness, numberOfInputImages, txtOutput, compactnessThreshold);
			#else
				flux = findParasiteFluxAlt(parasiteCompactness, numberOfInputImages, csvOutput, compactnessThreshold);
			#endif
			//flux = findParasiteFluxCompactness(numberOfInputImages, parasiteCompactness);
			//flux = findParasiteFlux3(numberOfInputImages, parasiteSize);
			#ifndef TXT
			#ifndef CSV
				printf("___________________________________\n");
				printf("Number of head projections: %d\n", flux);
			#endif
			#endif

			#ifdef TXT
				fprintf(txtOutput, "___________________________________\n");
				fprintf(txtOutput, "Number of head projections\t%d\n", flux);
			#endif

			#ifdef CSV
				fprintf(csvOutput, "___________________________________\n");
				fprintf(csvOutput, "Number of head projections,%d\n", flux);
			#endif

			printf("Number of head projections: %d\n", flux);
		}
		else
			printf("Not enough images to analyse.\n");

		#ifdef TXT
			printf("Results appended to file.\n");
			fprintf(txtOutput, "\n");
			fclose(txtOutput);
		#endif

		#ifdef CSV
			printf("Results appended to file.\n");
			fprintf(csvOutput, "\n");
			fclose(csvOutput);
		#endif
	#endif

	//free remaining global memory
	#ifdef CAL_SIZE
		free(parasiteSize);
	#endif
	free(parasiteCompactness);
	free(imagePath);
	
	printf("___________________\n");
	printf("PROGRAM COMPLETED\n\n");
	printf("PRESS ANY KEY TO EXIT\n");

	fflush(stdout);
	//pause the output stream
	//getchar();
	#ifndef RELEASE_BUILD
		getchar();
	#endif
	return EXIT_SUCCESS;
}


void processImages(struct threadStats* s)
{
	// Each thread will extract its threadID, the total number of images it must process and the 
	// image offset that it start at
	int threadID = (int)(struct threadStats*)(s)->id;
	int totalThreadImages = (int)(struct threadStats*)(s)->imagesPerThread;
	int threadImageOffset = (int)(struct threadStats*)(s)->imageOffset;

	unsigned char processorMask = 0x01;
	processorMask = processorMask << threadID;
	printf("Thread %d gets processor %d with %d images\n", threadID, threadID, totalThreadImages);
	
	// Calculate the starting image offset for the current thread
	int imageIndex = threadImageOffset;
	//int imageIndex = 11;

	// Cannot set affinity before creating thread.
	// SetThreadAffinityMask uses a bitmask for assigning processor cores.
	if (!SetThreadAffinityMask(GetCurrentThread(), (DWORD)processorMask)) { 
		printf("Could not set affinity\n");
		printf("Err: %d\n", GetLastError());
		//getchar();
		_endthread();
	}

	/* To double check processor affinity
		//DWORD proNum = 9;
		//printf("proNum B: %d\n", proNum);
		//proNum = GetCurrentProcessorNumber(); // returns a number between 0..N
		//printf("proNum A: %d\n", proNum);
	*/

	/* Small test to check processor affinity

		int o=0;
		int count = 0;
		while(o<20000000)
		{
			t_bmp* img = (t_bmp*)malloc(sizeof(t_bmp));
			free(img);
			o++;
		}

		_endthread();
	*/
	
	unsigned char* imageArray = (unsigned char *)malloc(sizeof(unsigned char) * MAX_DATA_SIZE);
	t_bmp* img = (t_bmp*)malloc(sizeof(t_bmp));

	char* imageFileName = (char*)malloc(sizeof(char)*255);
	memset(imageFileName, 0, sizeof(char)*255);
	sprintf(imageFileName,"%s/colour%d.bmp", imagePath, (imageIndex+1));
	//printf("imageFileName: %s\n", imageFileName);
	//printf("%d\tThread is Running\t%d: output file %s\n", threadID, totalThreadImages, imageFileName);

	// Create benchmarking output
	#ifdef BENCHMARKING
		#ifdef ELAPSED
		FILE *benchMarkFileElapsed;
		char outputName [30];
		sprintf (outputName, "benchMarkFileElapsed Thread_%d.txt", threadID);
		benchMarkFileElapsed = fopen(outputName, "w");
		fprintf(benchMarkFileElapsed, "------------Benchmarking Algorithms (Elapsed)------------\n");
		fprintf(benchMarkFileElapsed, "Thread ID %d.\n", threadID);
		fprintf(benchMarkFileElapsed, "Processor ID (start at 0) %d. \n", GetCurrentProcessorNumber());
		fprintf(benchMarkFileElapsed, "Number of Images %d.\n", totalThreadImages);
		fprintf(benchMarkFileElapsed, "Average elapsed times in ms.\n");
		fprintf(benchMarkFileElapsed, "Number of Passes:%d\n\n", NUM_OF_PASSES);
		double aggregatedElapsedTime = 0;
		#else
		FILE *benchMarkFileCPU;
		char outputName [30];
		sprintf (outputName, "benchMarkFileCPU Thread_%d.txt", threadID);
		benchMarkFileCPU = fopen(outputName, "w");
		fprintf(benchMarkFileCPU, "------------Benchmarking Algorithms (CPU)------------\n");
		fprintf(benchMarkFileElapsed, "Thread ID %d.\n", threadID);
		fprintf(benchMarkFileElapsed, "Processor ID %d.\n", GetCurrentProcessorNumber());
		fprintf(benchMarkFileElapsed, "Number of Images %d.\n", totalThreadImages);
		fprintf(benchMarkFileCPU, "Average CPU times in ms.\n");
		fprintf(benchMarkFileCPU, "Number of Passes:%d\n\n", NUM_OF_PASSES);
		double aggregatedCPUTime = 0;
		#endif
	#endif

	// While there are no more images to load.
	// (imageIndex-1) = starting image index
	// ((threadID*totalThreadImages)+totalThreadImages)) = final image index
	while((imageIndex<(threadImageOffset+totalThreadImages)) && (libbmp_load(imageFileName, img) != 0))
	//while(libbmp_load(imageFileName, img) != 0)
	{
		#ifdef TITLES
		printf("------ Processing Frame ------\n");
		#endif
		//printf("THREAD %d IMAGE %d\n", threadID, imageIndex);

		// Moy is black and white intensity
		// moy of 0 is black
		// moy of 255 is white
		// height goes from bottom up not top down
		// initialize all necessary variables
		////////////////////////////////////////////////////////////////////////////////
		/* Create Histogram                                                           */
		////////////////////////////////////////////////////////////////////////////////
		short* histogram;
		histogram = createHistogram(img);

		#ifdef DEBUG_OUTPUT
			//libbmp_write("image.bmp", img);
			libbmp_write("image.bmp", img);
		#endif
		////////////////////////////////////////////////////////////////////////////////
		/* End Create Histogram                                                       */
		////////////////////////////////////////////////////////////////////////////////

		/**************************************************************************************************************/

		////////////////////////////////////////////////////////////////////////////////
		/* Enhance Contrast                                                           */
		////////////////////////////////////////////////////////////////////////////////

		#ifdef TITLES
		printf("-------------------Enhance Contrast\n");
		#endif

		#ifdef BENCHMARKING
			t_bmp* initialImg;
			initialImg = (t_bmp*) malloc(sizeof(t_bmp));
			libbmp_copyAndCreateImg(img, initialImg);
		
			#ifdef ELAPSED
			stopWatch s;
			if(initialiseTimer(&s)==0) {
			#endif
				for (int i = 0; i < NUM_OF_PASSES; i++) {
					libbmp_copyImgData(initialImg, img);

					#ifdef ELAPSED
					startTimer(&s, processorMask);
					#else
					initCPUTime();
					#endif
		#endif

					enhanceContrast(img, histogram);
				
		#ifdef BENCHMARKING
					#ifdef ELAPSED
					stopTimer(&s, processorMask);
					#else
					double endTime = getCPUTimeSinceStart();
					#endif

					#ifdef ELAPSED
					printf("Elapsed Time: %f(s), %f(ms)\n", getElapsedTime(&s), getElapsedTimeInMilli(&s));
					aggregatedElapsedTime += getElapsedTimeInMilli(&s);
					#else
					printf("CPU Time: %f(ms)\n", endTime);
					aggregatedCPUTime += endTime;
					#endif
				}
			#ifdef ELAPSED
			} else { printf("Enhance constrast timer didnt initialise\n"); }
			#endif
		

			// Print results to file and screen and clean up
			#ifdef ELAPSED
			fprintf(benchMarkFileElapsed, "Enhance Contrast       : ");
			fprintf(benchMarkFileElapsed, "%f\n", (aggregatedElapsedTime/NUM_OF_PASSES));

			printf("\nAverage Elapsed Time: %f(ms)\n\n", (aggregatedElapsedTime/NUM_OF_PASSES));
			aggregatedElapsedTime = 0.0;
			#else
			fprintf(benchMarkFileCPU, "Enhance Contrast       : ");
			fprintf(benchMarkFileCPU, "%f\n", (aggregatedCPUTime/NUM_OF_PASSES));

			printf("\nAverage CPU Time: %f(ms)\n\n", (aggregatedCPUTime/NUM_OF_PASSES));
			aggregatedCPUTime = 0.0;
			#endif
		
			for(int i = 0; i < initialImg->height; i++)
				free(initialImg->data[i]);

			free(initialImg->data);
			free(initialImg);
		#endif

		// free histogram data structure
		free(histogram);

		#ifdef DEBUG_OUTPUT
			//enhanceContrast(img, histogram);
			//libbmp_write("imageEnhanceContrast.bmp", img);
			libbmp_write("imageStage1.bmp", img);
		#endif

		////////////////////////////////////////////////////////////////////////////////
		/* End Enhance Contrast                                                       */
		////////////////////////////////////////////////////////////////////////////////

		/**************************************************************************************************************/

		////////////////////////////////////////////////////////////////////////////////
		/* Filtering                                                                  */
		////////////////////////////////////////////////////////////////////////////////

		#ifdef TITLES
		printf("-------------------Filtering\n");
		#endif

		t_bmp* outputImg = (t_bmp*)malloc(sizeof(t_bmp));
		libbmp_copyAndCreateImg(img, outputImg);		// initialise outputImage by creating and copying

		#ifdef BENCHMARKING
			initialImg = (t_bmp*) malloc(sizeof(t_bmp));
			libbmp_copyAndCreateImg(img, initialImg);

			/* Updated the low pass filter sequential to take into account the border pixels.
			   Also removed a major bug where the convolution operation output the results of each pixel calculation to the same image instead of a new image.
		     */

			#ifdef ELAPSED
			if(initialiseTimer(&s)==0) {
			#endif
				for (int i = 0; i < NUM_OF_PASSES; i++) {
					libbmp_copyImgData(initialImg, img);

					#ifdef ELAPSED
					startTimer(&s, processorMask);
					#else
					initCPUTime();
					#endif
		#endif

					lowPassFilterImage(img, outputImg);
					//lowPassFilterImage(img);
		
		#ifdef BENCHMARKING
					#ifdef ELAPSED
					stopTimer(&s, processorMask);
					#else
					double endTime = getCPUTimeSinceStart();
					#endif

					#ifdef ELAPSED
					printf("Elapsed Time: %f(s), %f(ms)\n", getElapsedTime(&s), getElapsedTimeInMilli(&s));
					aggregatedElapsedTime += getElapsedTimeInMilli(&s);
					#else
					printf("CPU Time: %f(ms)\n", endTime);
					aggregatedCPUTime += endTime;
					#endif
				}
			#ifdef ELAPSED
			} else { printf("Filtering timer didnt initialise\n"); }
			#endif

			#ifdef ELAPSED
			fprintf(benchMarkFileElapsed, "Filtering              : ");
			fprintf(benchMarkFileElapsed, "%f\n", (aggregatedElapsedTime/NUM_OF_PASSES));

			printf("\nAverage Elapsed Time: %f(ms)\n\n", (aggregatedElapsedTime/NUM_OF_PASSES));
			aggregatedElapsedTime = 0.0;
			#else
			fprintf(benchMarkFileCPU, "Filtering              : ");
			fprintf(benchMarkFileCPU, "%f\n", (aggregatedCPUTime/NUM_OF_PASSES));

			printf("\nAverage CPU Time: %f(ms)\n\n", (aggregatedCPUTime/NUM_OF_PASSES));
			aggregatedCPUTime = 0.0;
			#endif

			for(int i = 0; i < initialImg->height; i++)
				free(initialImg->data[i]);

			free(initialImg->data);
			free(initialImg);
		#endif


		
		// Switch pointers so img points to outputImg (the result of the low pass filter operation) and outputImg points to the old data (img), which is then removed.
		t_bmp* temp;
		//printf("img: %x OutputImage: %x temp: %xd\n\n", img, outputImg, temp);
		temp = outputImg;
		//printf("temp = outputImage\n");
		//printf("img: %x OutputImage: %x temp: %xd\n", img, outputImg, temp);
		outputImg = img;
		//printf("outputImage = img\n");
		//printf("img: %x OutputImage: %x temp: %xd\n", img, outputImg, temp);
		img = temp;
		//printf("img = temp\n");
		//printf("img: %x OutputImage: %x temp: %xd\n", img, outputImg, temp);
		for(int i = 0; i < outputImg->height; i++)
			free(outputImg->data[i]);

		free(outputImg->data);
		free(outputImg);
		


		#ifdef DEBUG_OUTPUT
			//lowPassFilterImage(img);
			//medianFilterImage(img);
			//libbmp_write("imageMedianFilter.bmp", img);
			libbmp_write("imageStage2.bmp", img);
		#endif

		////////////////////////////////////////////////////////////////////////////////
		/* End Filtering                                                              */
		////////////////////////////////////////////////////////////////////////////////

		/**************************************************************************************************************/

		////////////////////////////////////////////////////////////////////////////////
		/* Segment Image                                                              */
		////////////////////////////////////////////////////////////////////////////////

		#ifdef TITLES
		printf("-------------------Segment Image\n");
		#endif
		
		#ifdef BENCHMARKING
			initialImg = (t_bmp*) malloc(sizeof(t_bmp));
			libbmp_copyAndCreateImg(img, initialImg);

			#ifdef ELAPSED
			if(initialiseTimer(&s)==0) {
			#endif
				for (int i = 0; i < NUM_OF_PASSES; i++) {
					libbmp_copyImgData(initialImg, img);

					#ifdef ELAPSED
					startTimer(&s, processorMask);
					#else
					initCPUTime();
					#endif
		#endif

					segmentImage(img);

		#ifdef BENCHMARKING
					#ifdef ELAPSED
					stopTimer(&s, processorMask);
					#else
					double endTime = getCPUTimeSinceStart();
					#endif

					#ifdef ELAPSED
					printf("Elapsed Time: %f(s), %f(ms)\n", getElapsedTime(&s), getElapsedTimeInMilli(&s));
					aggregatedElapsedTime += getElapsedTimeInMilli(&s);
					#else
					printf("CPU Time: %f(ms)\n", endTime);
					aggregatedCPUTime += endTime;
					#endif
				}
			#ifdef ELAPSED
			} else { printf("Segment timer didnt initialise\n"); }
			#endif

			#ifdef ELAPSED
			fprintf(benchMarkFileElapsed, "Segment Image          : ");
			fprintf(benchMarkFileElapsed, "%f\n", (aggregatedElapsedTime/NUM_OF_PASSES));
			printf("\nAverage Elapsed Time: %f(ms)\n\n", (aggregatedElapsedTime/NUM_OF_PASSES));
			aggregatedElapsedTime = 0.0;
			#else
			fprintf(benchMarkFileCPU, "Segment Image          : ");
			fprintf(benchMarkFileCPU, "%f\n", (aggregatedCPUTime/NUM_OF_PASSES));
			printf("\nAverage CPU Time: %f(ms)\n\n", (aggregatedCPUTime/NUM_OF_PASSES));
			aggregatedCPUTime = 0.0;
			#endif
		
			for(int i = 0; i < initialImg->height; i++)
				free(initialImg->data[i]);

			free(initialImg->data);
			free(initialImg);
		#endif

		#ifdef DEBUG_OUTPUT
			//segmentImage(img);
			//libbmp_write("imageSegmented.bmp", img);
			libbmp_write("imageStage3.bmp", img);
		#endif

		////////////////////////////////////////////////////////////////////////////////
		/* End Segment Image                                                          */
		////////////////////////////////////////////////////////////////////////////////

		/**************************************************************************************************************/

		//Convert image to a 1D array - required for dilation, erosion and noise removal
		imageToArray(img, imageArray);

		#ifdef DEBUG_OUTPUT
			createImage(img, imageArray);
			//libbmp_write("imageWith1DArray.bmp", img);
			libbmp_write("imageStage4.bmp", img);
		#endif

		/**************************************************************************************************************/

		////////////////////////////////////////////////////////////////////////////////
		/* Dilate Image three times                                                   */
		////////////////////////////////////////////////////////////////////////////////

		#ifdef TITLES
		printf("-------------------Dilate Image\n");
		#endif

		#ifdef BENCHMARKING
			unsigned char* initialImgArray;
			initialImgArray = (unsigned char *)malloc(sizeof(unsigned char) * MAX_DATA_SIZE);
			copyArray(imageArray, initialImgArray);

			#ifdef ELAPSED
			if(initialiseTimer(&s)==0) {
			#endif
				for (int i = 0; i < NUM_OF_PASSES; i++) {
					copyArray(initialImgArray, imageArray);

					#ifdef ELAPSED
					startTimer(&s, processorMask);
					#else
					initCPUTime();
					#endif
		#endif

					dilateImage(img->height, img->width, imageArray);
					dilateImage(img->height, img->width, imageArray);
					dilateImage(img->height, img->width, imageArray);
					dilateImage(img->height, img->width, imageArray);
					dilateImage(img->height, img->width, imageArray);
					dilateImage(img->height, img->width, imageArray);

		#ifdef BENCHMARKING
					#ifdef ELAPSED
					stopTimer(&s, processorMask);
					#else
					double endTime = getCPUTimeSinceStart();
					#endif

					#ifdef ELAPSED
					printf("Elapsed Time: %f(s), %f(ms)\n", getElapsedTime(&s), getElapsedTimeInMilli(&s));
					aggregatedElapsedTime += getElapsedTimeInMilli(&s);
					#else
					printf("CPU Time: %f(ms)\n", endTime);
					aggregatedCPUTime += endTime;
					#endif
				}
			#ifdef ELAPSED
			} else { printf("Dilate timer didnt initialise\n"); }
			#endif
		
			#ifdef ELAPSED
			fprintf(benchMarkFileElapsed, "Dilate Image           : ");
			fprintf(benchMarkFileElapsed, "%f\n", (aggregatedElapsedTime/NUM_OF_PASSES));
			printf("\nAverage Elapsed Time: %f(ms)\n\n", (aggregatedElapsedTime/NUM_OF_PASSES));
			aggregatedElapsedTime = 0.0;
			#else
			fprintf(benchMarkFileCPU, "Dilate Image           : ");
			fprintf(benchMarkFileCPU, "%f\n", (aggregatedCPUTime/NUM_OF_PASSES));
			printf("\nAverage CPU Time: %f(ms)\n\n", (aggregatedCPUTime/NUM_OF_PASSES));
			aggregatedCPUTime = 0.0;
			#endif	

			free(initialImgArray);
		#endif

		#ifdef DEBUG_OUTPUT
			createImage(img, imageArray);
			//libbmp_write("imageDilated.bmp", img);
			libbmp_write("imageStage5.bmp", img);
		#endif

		////////////////////////////////////////////////////////////////////////////////
		/* End Dilate Image three times                                               */
		////////////////////////////////////////////////////////////////////////////////

		// The following code was a template for creating a method to fill in the black gaps
		// in the worm segmentation.

		//int *fillDetails = findLargestArea(img->height, img->width, imageArray);
		////fill(imageArray, fillDetails[1], fillDetails[2]);

		//createImage(img, imageArray);
		//libbmp_write("imageStage5i.bmp", img);

		//free(fillDetails);

		/**************************************************************************************************************/

		////////////////////////////////////////////////////////////////////////////////
		/* Erode Image three times                                                    */
		////////////////////////////////////////////////////////////////////////////////

		#ifdef TITLES
		printf("-------------------Erode Image\n");
		#endif

		#ifdef BENCHMARKING
			initialImgArray = (unsigned char *)malloc(sizeof(unsigned char) * MAX_DATA_SIZE);
			copyArray(imageArray, initialImgArray);

			#ifdef ELAPSED
			if(initialiseTimer(&s)==0) {
			#endif
				for (int i = 0; i < NUM_OF_PASSES; i++) {
					copyArray(initialImgArray, imageArray);

					#ifdef ELAPSED
					startTimer(&s, processorMask);
					#else
					initCPUTime();
					#endif
		#endif

					erodeImage(img->height, img->width, imageArray);
					erodeImage(img->height, img->width, imageArray);
					erodeImage(img->height, img->width, imageArray);
					erodeImage(img->height, img->width, imageArray);
					erodeImage(img->height, img->width, imageArray);
					erodeImage(img->height, img->width, imageArray);

		#ifdef BENCHMARKING
					#ifdef ELAPSED
					stopTimer(&s, processorMask);
					#else
					double endTime = getCPUTimeSinceStart();
					#endif

					#ifdef ELAPSED
					printf("Elapsed Time: %f(s), %f(ms)\n", getElapsedTime(&s), getElapsedTimeInMilli(&s));
					aggregatedElapsedTime += getElapsedTimeInMilli(&s);
					#else
					printf("CPU Time: %f(ms)\n", endTime);
					aggregatedCPUTime += endTime;
					#endif
				}
			#ifdef ELAPSED
			} else { printf("Erode timer didnt initialise\n"); }
			#endif

			#ifdef ELAPSED
			fprintf(benchMarkFileElapsed, "Erode Image            : ");
			fprintf(benchMarkFileElapsed, "%f\n", (aggregatedElapsedTime/NUM_OF_PASSES));

			printf("\nAverage Elapsed Time: %f(ms)\n\n", (aggregatedElapsedTime/NUM_OF_PASSES));
			aggregatedElapsedTime = 0.0;
			#else
			fprintf(benchMarkFileCPU, "Erode Image            : ");
			fprintf(benchMarkFileCPU, "%f\n", (aggregatedCPUTime/NUM_OF_PASSES));

			printf("\nAverage CPU Time: %f(ms)\n\n", (aggregatedCPUTime/NUM_OF_PASSES));
			aggregatedCPUTime = 0.0;
			#endif

			free(initialImgArray);
		#endif
		
		#ifdef DEBUG_OUTPUT
			createImage(img, imageArray);
			//libbmp_write("imageEroded.bmp", img);
			libbmp_write("imageStage6.bmp", img);
		#endif

		////////////////////////////////////////////////////////////////////////////////
		/* Erode Image three times                                                    */
		////////////////////////////////////////////////////////////////////////////////

		/**************************************************************************************************************/

		////////////////////////////////////////////////////////////////////////////////
		/* Remove Noise                                                               */
		////////////////////////////////////////////////////////////////////////////////

		#ifdef TITLES
		printf("-------------------Remove Noise\n");
		#endif

		int maskSize = 3;

		#ifdef BENCHMARKING
			initialImgArray = (unsigned char *)malloc(sizeof(unsigned char) * MAX_DATA_SIZE);
			copyArray(imageArray, initialImgArray);

			#ifdef ELAPSED
			if(initialiseTimer(&s)==0) {
			#endif
				for (int i = 0; i < NUM_OF_PASSES; i++) {
					copyArray(initialImgArray, imageArray);

					#ifdef ELAPSED
					startTimer(&s, processorMask);
					#else
					initCPUTime();
					#endif
		#endif

					removeUnwantedNoiseHeight(img->height, img->width, imageArray, maskSize);
					removeUnwantedNoiseWidth(img->height, img->width, imageArray, maskSize);
					removeUnwantedNoiseHeight(img->height, img->width, imageArray, maskSize);
					removeUnwantedNoiseWidth(img->height, img->width, imageArray, maskSize);

		#ifdef BENCHMARKING
					#ifdef ELAPSED
					stopTimer(&s, processorMask);
					#else
					double endTime = getCPUTimeSinceStart();
					#endif

					#ifdef ELAPSED
					printf("Elapsed Time: %f(s), %f(ms)\n", getElapsedTime(&s), getElapsedTimeInMilli(&s));
					aggregatedElapsedTime += getElapsedTimeInMilli(&s);
					#else
					printf("CPU Time: %f(ms)\n", endTime);
					aggregatedCPUTime += endTime;
					#endif
				}
			#ifdef ELAPSED
			} else { printf("Unwanted noise timer didnt initialise\n"); }
			#endif

			#ifdef ELAPSED
			fprintf(benchMarkFileElapsed, "Remove Noise           : ");
			fprintf(benchMarkFileElapsed, "%f\n", (aggregatedElapsedTime/NUM_OF_PASSES));

			printf("\nAverage Elapsed Time: %f(ms)\n\n", (aggregatedElapsedTime/NUM_OF_PASSES));
			aggregatedElapsedTime = 0.0;
			#else
			fprintf(benchMarkFileCPU, "Remove Noise           : ");
			fprintf(benchMarkFileCPU, "%f\n", (aggregatedCPUTime/NUM_OF_PASSES));

			printf("\nAverage CPU Time: %f(ms)\n\n", (aggregatedCPUTime/NUM_OF_PASSES));
			aggregatedCPUTime = 0.0;
			#endif

			free(initialImgArray);
		#endif
		
		#ifdef DEBUG_OUTPUT
			createImage(img, imageArray);
			//libbmp_write("imageNoiseRemoval.bmp", img);
			libbmp_write("imageStage7.bmp", img);
		#endif

		////////////////////////////////////////////////////////////////////////////////
		/* End Remove Noise                                                           */
		////////////////////////////////////////////////////////////////////////////////

		/**************************************************************************************************************/

		////////////////////////////////////////////////////////////////////////////////
		/* Find Centre Points                                                         */
		////////////////////////////////////////////////////////////////////////////////

		//getLineSizeWidth(img->height, img->width, imageArray);
		//getLineSizeHeight(img->height, img->width, imageArray);
		//createImage(img, imageArray);
		//libbmp_write("imageCentrePoints.bmp", img);

		////////////////////////////////////////////////////////////////////////////////
		/* End Find Centre Points                                                     */
		////////////////////////////////////////////////////////////////////////////////

		/**************************************************************************************************************/

		////////////////////////////////////////////////////////////////////////////////
		/* Calculate Area                                                             */
		////////////////////////////////////////////////////////////////////////////////

		#ifdef COMPACTNESS
			#ifdef TITLES
			printf("-------------------Find Pixels\n");
			#endif

			//libbmp_load("images/colourprocessed1CircleTest.bmp", img);
			//imageToArray(img, imageArray);

			int *details = (int*) malloc(sizeof(int) * 3);

			#ifdef BENCHMARKING
				initialImgArray = (unsigned char *)malloc(sizeof(unsigned char) * MAX_DATA_SIZE);
				copyArray(imageArray, initialImgArray);

				#ifdef ELAPSED
				if(initialiseTimer(&s)==0) {
				#endif
					for (int i = 0; i < NUM_OF_PASSES; i++) {
						copyArray(initialImgArray, imageArray);

						#ifdef ELAPSED
						startTimer(&s, processorMask);
						#else
						initCPUTime();
						#endif
			#endif

					free(details);
					details = findPixels(img->height, img->width, imageArray);

			#ifdef BENCHMARKING
						#ifdef ELAPSED
						stopTimer(&s, processorMask);
						#else
						double endTime = getCPUTimeSinceStart();
						#endif

						#ifdef ELAPSED
						printf("Elapsed Time: %f(s), %f(ms)\n", getElapsedTime(&s), getElapsedTimeInMilli(&s));
						aggregatedElapsedTime += getElapsedTimeInMilli(&s);
						#else
						printf("CPU Time: %f(ms)\n", endTime);
						aggregatedCPUTime += endTime;
						#endif
					}
				#ifdef ELAPSED
				} else { printf("Find pixels timer didnt initialise\n"); }
				#endif

				#ifdef ELAPSED
				fprintf(benchMarkFileElapsed, "Find Pixels            : ");
				fprintf(benchMarkFileElapsed, "%f\n", (aggregatedElapsedTime/NUM_OF_PASSES));

				printf("\nAverage Elapsed Time: %f(ms)\n\n", (aggregatedElapsedTime/NUM_OF_PASSES));
				aggregatedElapsedTime = 0.0;
				#else
				fprintf(benchMarkFileCPU, "Find Pixels            : ");
				fprintf(benchMarkFileCPU, "%f\n", (aggregatedCPUTime/NUM_OF_PASSES));

				printf("\nAverage CPU Time: %f(ms)\n\n", (aggregatedCPUTime/NUM_OF_PASSES));
				aggregatedCPUTime = 0.0;
				#endif

				free(initialImgArray);
			#endif

			//createImage(img, imageArray);
			//libbmp_write("imageStage7i.bmp", img);
			
			
			//details = findPixels(img->height, img->width, imageArray);
			//printf("**************\n");
			//printf("details[0] %d : details[1] %d : details[2] %d\n", details[0], details[1], details[2]); // largestArea, blobCoordI, blobCoordJ
			//printf("**************\n");

			#ifdef TITLES
			printf("-------------------Find and Replace Pixels\n");
			#endif

			#ifdef BENCHMARKING
				initialImgArray = (unsigned char *)malloc(sizeof(unsigned char) * MAX_DATA_SIZE);
				copyArray(imageArray, initialImgArray);

				//outputArrays(imageArray, "a", initialImgArray, "b");

				#ifdef ELAPSED
				if(initialiseTimer(&s)==0) {
				#endif
					for (int i = 0; i < NUM_OF_PASSES; i++) {
						copyArray(initialImgArray, imageArray);

						#ifdef ELAPSED
						startTimer(&s, processorMask);
						#else
						initCPUTime();
						#endif
			#endif
						//printf("details[0] %d : details[1] %d : details[2] %d\n", details[0], details[1], details[2]); // largestArea, blobCoordI, blobCoordJ
						findAndReplace(imageArray, details[1], details[2], 4, 1);

			#ifdef BENCHMARKING
						#ifdef ELAPSED
						stopTimer(&s, processorMask);
						#else
						double endTime = getCPUTimeSinceStart();
						#endif

						#ifdef ELAPSED
						printf("Elapsed Time: %f(s), %f(ms)\n", getElapsedTime(&s), getElapsedTimeInMilli(&s));
						aggregatedElapsedTime += getElapsedTimeInMilli(&s);
						#else
						printf("CPU Time: %f(ms)\n", endTime);
						aggregatedCPUTime += endTime;
						#endif
					}
				#ifdef ELAPSED
				} else { printf("Find and replace timer didnt initialise\n"); }
				#endif

				//outputArrays(imageArray, "e", initialImgArray, "f");

				#ifdef ELAPSED
				fprintf(benchMarkFileElapsed, "Find and Replace Pixels: ");
				fprintf(benchMarkFileElapsed, "%f\n", (aggregatedElapsedTime/NUM_OF_PASSES));

				printf("\nAverage Elapsed Time: %f(ms)\n\n", (aggregatedElapsedTime/NUM_OF_PASSES));
				aggregatedElapsedTime = 0.0;
				#else
				fprintf(benchMarkFileCPU, "Find and Replace Pixels: ");
				fprintf(benchMarkFileCPU, "%f\n", (aggregatedCPUTime/NUM_OF_PASSES));

				printf("\nAverage CPU Time: %f(ms)\n\n", (aggregatedCPUTime/NUM_OF_PASSES));
				aggregatedCPUTime = 0.0;
				#endif

				free(initialImgArray);
				free(details);
			#endif
		
			double area;
			double perimiter;
			double compactness;

			#ifdef BENCHMARKING
				initialImgArray = (unsigned char *)malloc(sizeof(unsigned char) * MAX_DATA_SIZE);
				copyArray(imageArray, initialImgArray);

				#ifdef ELAPSED
				if(initialiseTimer(&s)==0) {
				#endif
					for (int i = 0; i < NUM_OF_PASSES; i++) {
						copyArray(initialImgArray, imageArray);

						#ifdef ELAPSED
						startTimer(&s, processorMask);
						#else
						initCPUTime();
						#endif
			#endif
							area = calculateArea(imageArray);
							//perimiter = calculatePerimeter(img, imageArray);
							perimiter = calculatePerimeterOptimised(img, imageArray, area);
							compactness = calculateCompactness(perimiter, area);
							//printf("Area: %g \n", area);
							//printf("Perimiter: %g \n", perimiter);
							//printf("Compactness: %g \n", (double) compactness);

			#ifdef BENCHMARKING
						#ifdef ELAPSED
						stopTimer(&s, processorMask);
						#else
						double endTime = getCPUTimeSinceStart();
						#endif

						#ifdef ELAPSED
						printf("Elapsed Time: %f(s), %f(ms)\n", getElapsedTime(&s), getElapsedTimeInMilli(&s));
						aggregatedElapsedTime += getElapsedTimeInMilli(&s);

						//fprintf(benchMarkFileElapsed, "%f\n", getElapsedTimeInMilli(&s));
						#else
						printf("CPU Time: %f(ms)\n", endTime);
						aggregatedCPUTime += endTime;
						#endif
					}
				#ifdef ELAPSED
				} else { printf("Compactness timer didnt initialise\n"); }
				#endif


				#ifdef ELAPSED
				fprintf(benchMarkFileElapsed, "Calculate Compactness  : ");
				fprintf(benchMarkFileElapsed, "%f\n", (aggregatedElapsedTime/NUM_OF_PASSES));

				printf("\nAverage Elapsed Time: %f(ms)\n\n", (aggregatedElapsedTime/NUM_OF_PASSES));
				aggregatedElapsedTime = 0.0;
				#else
				fprintf(benchMarkFileCPU, "Calculate Compactness: ");
				fprintf(benchMarkFileCPU, "%f\n", (aggregatedCPUTime/NUM_OF_PASSES));

				printf("\nAverage CPU Time: %f(ms)\n\n", (aggregatedCPUTime/NUM_OF_PASSES));
				aggregatedCPUTime = 0.0;
				#endif

				free(initialImgArray);
				//checkPixels(imageArray, details[1], details[2], 4, 1);
			#endif

			parasiteCompactness[imageIndex] = compactness;
			
			//// Calculate bounding box
			//findBoundingBox(img->height, img->width, imageArray);
			//createBinaryImage(img, imageArray);
			//char* boundingImageFileName = (char*)malloc(sizeof(char)*19);
			//strcpy(boundingImageFileName, "images/imageNoiseBounding");
			//strcat(boundingImageFileName, imageIndexString);
			//strcat(boundingImageFileName, ".bmp");
			//libbmp_write(boundingImageFileName, img);
		#endif

		////////////////////////////////////////////////////////////////////////////////
		/* End Calculate Area                                                         */
		////////////////////////////////////////////////////////////////////////////////

		/**************************************************************************************************************/


		////////////////////////////////////////////////////////////////////////////////
		/* Find Parasite Size                                                         */
		////////////////////////////////////////////////////////////////////////////////
		#ifdef CAL_SIZE
			#ifdef BENCHMARKING
				unsigned char* initialImgArray;
				initialImgArray = (unsigned char *)malloc(sizeof(unsigned char) * MAX_DATA_SIZE);
				copyArray(imageArray, initialImgArray);

				#ifdef ELAPSED
				if(initialiseTimer(&s)==0) {
				#endif
					for (int i = 0; i < NUM_OF_PASSES; i++) {
						copyArray(initialImgArray, imageArray);

						#ifdef ELAPSED
						startTimer(&s);
						#else
						initCPUTime();
						#endif
			#endif

						parasiteSize[imageIndex] = (short) getSizeOfBlobCircleTest(img->height, img->width, imageArray);

			#ifdef BENCHMARKING
						#ifdef ELAPSED
						stopTimer(&s);
						#else
						double endTime = getCPUTimeSinceStart();
						#endif

						#ifdef ELAPSED
						printf("Elapsed Time: %f(s), %f(ms)\n", getElapsedTime(&s), getElapsedTimeInMilli(&s));
						aggregatedElapsedTime += getElapsedTimeInMilli(&s);

						//fprintf(benchMarkFileElapsed, "%f\n", getElapsedTimeInMilli(&s));
						#else
						printf("CPU Time: %f(ms)\n", endTime);
						aggregatedCPUTime += endTime;
						#endif
					}
				#ifdef ELAPSED
				} else { printf("Find Blob size timer didnt initialise\n"); }
				#endif

				#ifdef ELAPSED
				fprintf(benchMarkFileElapsed, "Find blob size         : ");
				fprintf(benchMarkFileElapsed, "%f\n", (aggregatedElapsedTime/NUM_OF_PASSES));

				printf("\nAverage Elapsed Time: %f(ms)\n\n", (aggregatedElapsedTime/NUM_OF_PASSES));
				aggregatedElapsedTime = 0.0;
				#else
				fprintf(benchMarkFileCPU, "Find blob size         : ");
				fprintf(benchMarkFileCPU, "%f\n", (aggregatedCPUTime/NUM_OF_PASSES));

				printf("\nAverage CPU Time: %f(ms)\n\n", (aggregatedCPUTime/NUM_OF_PASSES));
				aggregatedCPUTime = 0.0;
				#endif

				free(initialImgArray);
			#endif
		#endif

		////////////////////////////////////////////////////////////////////////////////
		/* End Find Parasite Size                                                     */
		////////////////////////////////////////////////////////////////////////////////

		#ifdef DEBUG_OUTPUT
			// Redraw the noise removal image once the noise and fluke have been distinguished.
			createImage(img, imageArray);
			//libbmp_write("imageNoiseRemoval.bmp", img);
			libbmp_write("imageStage8.bmp", img);
		#endif

		/**************************************************************************************************************/

		#ifdef OUTPUT_PROCESSED
			createImage(img, imageArray);

			//strcpy(imageFileName, (char*) argv[1]);
			//strcat(imageFileName, "\\colour");
			//strcat(imageFileName, "processed");
			//strcat(imageFileName, imageIndexString);
			//strcat(imageFileName, ".bmp");

			//sprintf(imageFileName, "images/colourprocessed%d.bmp", (imageIndex+1));
			sprintf(imageFileName, "%s/colourprocessed%d.bmp", imagePath, (imageIndex+1));
			libbmp_write(imageFileName, img);
		#endif

		/**************************************************************************************************************/

		////////////////////////////////////////////////////////////////////////////////
		/* Create new image name for next image                                       */
		////////////////////////////////////////////////////////////////////////////////
		
		//printf("image index: %d \n", imageIndex);
		imageIndex++;
		//itoa(imageIndex,imageIndexString,10); // void itoa(int input, char *buffer, int radix) - base radix: 10 (decimal)

		//strcpy(imageFileName, "images/colour");
		//strcat(imageFileName, imageIndexString);
		//strcat(imageFileName, ".bmp");

		sprintf(imageFileName,"%s/colour%d.bmp", imagePath, (imageIndex+1));

		//strcpy(imageFileName, (char*) argv[1]);
		//strcat(imageFileName, "\\colour");
		//strcat(imageFileName, imageIndexString);
		//strcat(imageFileName, ".bmp");

		////////////////////////////////////////////////////////////////////////////////
		/* End Create new image name for next image                                   */
		////////////////////////////////////////////////////////////////////////////////

		#ifdef BENCHMARKING
			#ifdef ELAPSED
			fprintf(benchMarkFileElapsed, "\n");
			#else
			fprintf(benchMarkFileCPU, "\n");
			#endif
		#endif

		// Free all allocated image memory
		for(int i = 0; i < img->height; i++)
			free(img->data[i]);
		free(img->data);
	}

	#ifdef BENCHMARKING
		#ifdef ELAPSED
			fclose(benchMarkFileElapsed);
		#else
			fclose(benchMarkFileCPU);
		#endif
	#endif

	//printf("ThreadID %d cleaning up\n", threadID);
	//fflush(NULL);
	free(img);
	free(imageArray);
	free(imageFileName);
	

	// Kill the current thread once it has completed its work 
	//printf("ThreadID %d has ended\n", threadID);
	_endthread();

}
