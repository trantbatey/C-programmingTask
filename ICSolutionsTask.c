/*
 ============================================================================
 Name        : ICSolutionsTask.c
 Author      : Trant Batey
 Version     : V1
 Copyright   : Your copyright notice
 Description :
	Please write a computer program in C to run on a Linux computer using
	the GNU compiler as part of your technical skills assessment.

	Puzzle Description:

	A comma-delimited file has two columns: timeA, timeB.

	- Both columns are times in hh:mm [a|p]m (hh = 1-12, mm = 0-59)
	or ISO format (YYYY-MM-DD hh24:mm).

	- Times could be in either format, but for any given row the times
	will be in the same format.

	Write a program to read the file:

	   - For each line report the time that is earlier.

	Assume same day for each line, and also same timezone for each time.

	The records should be displayed in the order they are in the file.

	- After the initial print, print for each row, all the times that are
	later without reading the file again.

	The records should be displayed in reverse order of the file order.

	The program should use linked lists.

	Include any instructions necessary to build your program.

 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// constants
static const int FILE_PATH_SIZE = 1000;

/*
 * Structure for handling time of day
 */
struct TimeBlock {
   char  aTime[24];
   int   aTimeValue;
   char  bTime[24];
   int   bTimeValue;
};

/*
 * Link list Node
 */
struct TimeNode {
	struct TimeBlock *timeBlock;
	struct TimeNode  *next;
};

// proto types
FILE* openFile(char *fname);;
void cleanData(char *line, char *cleanLine);
void addDelimiter(char *line);
void printEarly(struct TimeBlock *timeBlock);
void printLate(struct TimeBlock *timeBlock);
int calculate12Time(char* hour, char *minute, char ampmFlag);
int calculate24Time(char* hour, char *minute);
void divideRows(char *dataLine, struct TimeBlock *timeBlock);
void get12HourTimeBlock(char *dataLine, struct TimeBlock *timeBlock);
void get24HourTimeBlock(char *dataLine, struct TimeBlock *timeBlock);
void itRecursToMe(struct TimeNode *node);
void printList(struct TimeNode *head);
void addToList(struct TimeBlock *timeBlock,
		struct TimeNode **headPPt,
		struct TimeNode **tailPPt);
void freeList(struct TimeNode *head);


// main driver function
int main(int argc, char **args) {

    // open file
    FILE* fp;
	if (argc >= 2) {
		// from command line
	    fp = fopen(args[1], "r");
	} else {
		// from user input
		char *fname = NULL;
		fp = openFile(fname);
	}

    // Checks error opening the file
    if (fp == NULL) {
    	printf("\nopenFile(): Unable to open the file for reading\n");
    	return 1;
    }

    // setup to read file
    size_t len = 0;
    ssize_t read;
    int iData;
	char *p;
    char *line = NULL;
    struct TimeNode *head = NULL;
    struct TimeNode *tail = NULL;

    // read file to build list
    struct TimeBlock *timeBlock = NULL;
    while ((read = getline(&line, &len, fp)) != -1) {
		timeBlock = malloc(sizeof(struct TimeBlock));
    	if ((p = strchr(line, '-')) == NULL) {
    		// 12 hour time
    		get12HourTimeBlock(line, timeBlock);
    	} else  {
    		// 24 hour time
    		get24HourTimeBlock(line, timeBlock);
    	}
    	addToList(timeBlock, &head, &tail);
    }

    // print the list
    printList(head);

    // free linked list
    freeList(head);

    // success
    fclose(fp);
    return 0;
}

/*
 * function to open a file based on the user entering the file name.
 * input:
 * 	char *fname - file name
 * 	return - file pointer
 * 		NULL means an error
 */
FILE* openFile(char *fname) {

	// enter file name
    FILE* fp;
	fname = (char *) malloc(sizeof(char) * FILE_PATH_SIZE);
	memset(fname, 0, FILE_PATH_SIZE);
	char *p;

	/*ask user for the name of the file*/
	printf("Enter file name: ");
	if (fgets(fname, FILE_PATH_SIZE - 1, stdin) == NULL) {
		printf("\nopenFile(): There was an error reading the file name.");
		return NULL;
	}
	if ((p = strchr(fname, '\n')) != NULL)
		*p = '\0';

	//Opens the file from where the text will be read.
	fp = fopen(fname, "r");
	free(fname);

	return fp;
}

/*
 * function to strip blanks
 * input:
 * 	char *line - line of data read from file
 * 	return - char*
 * 		line of clean data without blanks
 */
void cleanData(char *line, char *cleanLine) {
	if (line == NULL) return;
	int j = 0;
	for (int i = 0; i < strlen(line); i++) {
		if (line[i] == ' ') continue;
		cleanLine[j] = line[i];
		j++;
	}
	cleanLine[j] = '\0';
}

/*
 * function add a delimiter to ISO time
 * input:
 * 	char *line - line of data read from file
 * 	return - char*
 * 		ISO time with a colon to delimit the HHs and MMs
 */
void addDelimiter(char *line) {
	if (line == NULL) return;
	char *p = line;
	while (1) {
		p = strchr(p, '-');
		if (p == NULL) break;
		while (*p != ' ') p++;
		*p = ':';
	}
}

/*
 * function to print the early time
 * input:
 * 	struct TimeBlock timeBlock
 * 	return void
 */
void printEarly(struct TimeBlock *timeBlock) {
	if (timeBlock->aTimeValue < timeBlock->bTimeValue)
		printf("%s\n", timeBlock->aTime);
	else
		printf("%s\n", timeBlock->bTime);
}

/*
 * function to print the late time
 * input:
 * 	struct TimeBlock timeBlock
 * 	return void
 */
void printLate(struct TimeBlock *timeBlock) {
	if (timeBlock->aTimeValue > timeBlock->bTimeValue)
		printf("%s\n", timeBlock->aTime);
	else
		printf("%s\n", timeBlock->bTime);
}

/*
 * function to calculate a 12 hour time
 * input:
 *     char* hour
 *     char *minute
 *     char ampmFlag
 * 	return the number of minutes since midnight (00:00 hrs)
 */
int calculate12Time(char* hour, char *minute, char ampmFlag) {
	int iHour = atoi(hour);
	int iMinute = atoi(minute);
	if ((ampmFlag == 'a') || (ampmFlag == 'A')) {
		if (iHour == 12) iHour -= 12;
	} else {
		if (iHour < 12) iHour += 12;
	}
	int time = (iHour*60) + iMinute;
	return time;
}

/*
 * function to calculate a 24 hour time
 * input:
 *     char* hour
 *     char *minute
 * 	return the number of minutes since midnight (00:00 hrs)
 */
int calculate24Time(char* hour, char *minute) {
	int iHour = atoi(hour);
	int iMinute = atoi(minute);
	int time = (iHour*60) + iMinute;
	return time;
}

/*
 * function to create divide rows
 * input:
 *  char *dataLine
 * 	struct TimeBlock timeBlock
 * 	return void
 */
void divideRows(char *dataLine, struct TimeBlock *timeBlock) {

	// scan to comma
	int i = 0, j = 0;
	while(dataLine[i] != ',') {
		timeBlock->aTime[i] = dataLine[i];
		i++;
	}
	timeBlock->aTime[i] = '\0';
	i++;
	while(dataLine[i] != '\n') {
		timeBlock->bTime[j] = dataLine[i];
		i++;
		j++;
	}
	timeBlock->bTime[i] = '\0';
	return;
}

/*
 * function to create a 12 hour time block
 * input:
 *  char *dataLine
 * 	struct TimeBlock timeBlock
 * 	return void
 */
void get12HourTimeBlock(char *dataLine, struct TimeBlock *timeBlock) {

	// divide rows
	divideRows(dataLine, timeBlock);

	// 12 hour time
    char hour[4], minute[4], ampmFlag, dummy;
    char cleanLine[24];

    cleanData(timeBlock->aTime, cleanLine);
	sscanf(cleanLine, "%[0-9]%c%[0-9]%c%c",
			hour, &dummy, minute, &ampmFlag, &dummy);
	timeBlock->aTimeValue = calculate12Time(hour, minute, ampmFlag);

    cleanData(timeBlock->bTime, cleanLine);
	sscanf(cleanLine, "%[0-9]%c%[0-9]%c%c",
			hour, &dummy, minute, &ampmFlag, &dummy);
	timeBlock->bTimeValue = calculate12Time(hour, minute, ampmFlag);
}

/*
 * function to create a 24 hour time block
 * input:
 *  char *dataLine
 * 	struct TimeBlock timeBlock
 * 	return void
 */
void get24HourTimeBlock(char *dataLine, struct TimeBlock *timeBlock) {

	// divide rows
	divideRows(dataLine, timeBlock);

	// 24 hour time
    char hour[4], minute[4], dummy;
    char cleanLine[24], year[8], month[4], day[4];

    strcpy(cleanLine, timeBlock->aTime);
    addDelimiter(cleanLine);
    cleanData(cleanLine, cleanLine);
	sscanf(cleanLine, "%[0-9]%c%[0-9]%c%[0-9]%c%[0-9]%c%[0-9]",
			year, &dummy, month, &dummy, day, &dummy,
			hour, &dummy, minute);
	timeBlock->aTimeValue = calculate24Time(hour, minute);

    strcpy(cleanLine, timeBlock->bTime);
    addDelimiter(cleanLine);
    cleanData(cleanLine, cleanLine);
	sscanf(cleanLine, "%[0-9]%c%[0-9]%c%[0-9]%c%[0-9]%c%[0-9]",
			year, &dummy, month, &dummy, day, &dummy,
			hour, &dummy, minute);
	timeBlock->bTimeValue = calculate24Time(hour, minute);
}

// recursive list print and back
void itRecursToMe(struct TimeNode *node) {

	// if list is null something went wrong
	if (node == NULL) {
		// print reverse Header
		printf("\nPrinting the late times in reverse file order: \n\n");
		return;
	}

	// print early time
	printEarly(node->timeBlock);
	itRecursToMe(node->next);
	printLate(node->timeBlock);

}


/*
 * function print the linked list
 *
 * Normally you wouldn't do this recursively because it waste the stack,
 * but since this is just a practice program, I wanted to make it cooler.
 *
 * input:
 *  struct TimeNode *head
 * return void
 */
void printList(struct TimeNode *head) {

	// print the header
	printf("\nPrinting the early times in file order: \n\n");

	// recursive call
	itRecursToMe(head);

}

// Function adds node to list
void addToList(struct TimeBlock *timeBlock,
		struct TimeNode **headPPt,
		struct TimeNode **tailPPt) {
	struct TimeNode *head;
	struct TimeNode *tail;
	head = *headPPt;
	if (head == NULL) {
		head = malloc(sizeof(struct TimeNode));
		head->timeBlock  = timeBlock;
		head->next = NULL;
		*headPPt = head;
		tail = head;
	} else {
		tail = *tailPPt;
		tail->next = malloc(sizeof(struct TimeNode));
		tail = tail->next;
		tail->timeBlock = timeBlock;
		tail->next = NULL;
	}
	*tailPPt = tail;
}

// free linked list
void freeList(struct TimeNode *head) {
	struct TimeNode *tail = NULL;
	while (head != NULL) {
		tail = head;
		head = head->next;
		free(tail->timeBlock);
		free(tail);
	}
}
