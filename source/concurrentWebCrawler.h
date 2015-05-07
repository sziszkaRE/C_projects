/*
 * main.h
 *
 *  Created on: May 5, 2015
 *      Author: Szilard Ratting
 */

#ifndef CONCURRENTWEBCRAWLER_H_
#define CONCURRENTWEBCRAWLER_H_

/* Typedefs*/
typedef struct tag_myString
{
	char *string;
	size_t size;
}tMyString;

typedef struct tag_myList
{
	tMyString *listOfLinks;
	size_t numberOfLinks;
}tMyList;

typedef struct tag_threadData
{
	tMyString *link;
	int deep;
}tThreadData;

#endif /* CONCURRENTWEBCRAWLER_H_ */
