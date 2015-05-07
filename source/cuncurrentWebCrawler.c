#include <stdio.h>     /* due to prinft */
#include <stdlib.h>    /* due to exit */
#include <string.h>    /* due to the memcpy function*/
#include <curl/curl.h> /* due to HTML functions */
#include <pthread.h>   /* due to concurrency handling */
#include <regex.h>     /* due to parsing HTML */
#include "concurrentWebCrawler.h"


/* Constants definitions */
static const char *urlPattern = "http:\/\/?([\da-z\.-]+)\.([a-z\.]{2,6})([\/\w \.-]*)*\/?";

/* Global variables */
static tMyList g_listOfLinks;
static pthread_mutex_t mutexLinks;


/* Static function declarations. */
static size_t writeToBuffer(void *pContents, size_t pSize, size_t pNmemb, void *pUserp);
static int rxmatch(const char *pString, regmatch_t *result, const char *pPattern);
static void putLinkIntoList(tMyList *pList, tMyString *pLink);
static tMyString getLinkFromList(tMyList *pList, const int idx);
static void deleteList(tMyList *pList);
static void deleteBuffer(tMyString *pString);
static void processPage(char* pPage, tMyList* pLinkList);
static int openPage(const char* pPage, tMyString* outBuffer);
static void webcrawler(void *pThreadData);

/* Static function definitions. */
static void webcrawler(void *pThreadData)
{
    pthread_t *thread;
    pthread_attr_t attr;

    tMyString pageBuffer;
    tMyList listOfLinks;
    int openPageRetVal;
    int idx;
    int rc;
    tThreadData* threadData;
    void *status;

    /* Initialize and set thread detached attribute */
    thread = (pthread_t *) malloc(sizeof(pthread_t));

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    /* Init local variables */
    threadData = (tThreadData*)pThreadData;

    pageBuffer.size = 0u;
    pageBuffer.string = (char*)malloc( 1u * sizeof(char) );

    listOfLinks.numberOfLinks = 0u;
    listOfLinks.listOfLinks = (tMyString*)malloc( 1u * sizeof(tMyString) );

    openPageRetVal = 0u;
    idx = 0u;

    /* Do the work. */
    pthread_mutex_lock (&mutexLinks);
    putLinkIntoList(&g_listOfLinks, threadData->link);
    pthread_mutex_unlock (&mutexLinks);

    if( threadData->deep > 0u)
    {
        openPageRetVal = openPage(threadData->link->string, &pageBuffer);
        if(0 == openPageRetVal)
        {
            /* Save all links found in the opened page into list. */
            processPage(pageBuffer.string, &listOfLinks);
            if( listOfLinks.numberOfLinks > 0u )
            {
                threadData->deep -= 1u;
                for( idx = 0u; idx < listOfLinks.numberOfLinks; idx++ )
                {
                    threadData->link = &(listOfLinks.listOfLinks[idx]);
                    rc = pthread_create(thread,  &attr, webcrawler, (void *)threadData);
                    if(rc){
                        printf("Error code: %d\n", rc);
                    }

                    rc = pthread_join(*thread, 0u);
                    if(rc){
                        printf("Join error code: %d", rc);
                    }

                }/*for*/
                threadData->deep += 1u; /* current level is processed completely */
            }
        }
    } /*if*/

    deleteBuffer(&pageBuffer);
    deleteList(&listOfLinks);
    return;
}/* end of the webcrawler function */


static int openPage(const char* pPage, tMyString* outBuffer)
{
    CURL *conn;
    CURLcode code;
    int retCode = 0u;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    conn = curl_easy_init();
    if(conn == NULL)
    {
        retCode = 1u;
        return retCode;
    }
    code = curl_easy_setopt(conn, CURLOPT_URL, pPage);
    if(code != CURLE_OK)
    {
        retCode = 2u;
        return retCode;
    }
    code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
    if (code != CURLE_OK)
    {
        retCode = 3u;
        return retCode;
    }
    code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writeToBuffer);
    if (code != CURLE_OK)
    {
        retCode = 4u;
        return retCode;
    }
    code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, (void *)outBuffer);
    if (code != CURLE_OK)
    {
        retCode = 4u;
        return retCode;
    }
    code = curl_easy_perform(conn);
    if (code != CURLE_OK)
    {
        retCode = 5u;
        return retCode;
    }

    curl_easy_cleanup(conn);

    return retCode;
}

static void processPage(char* pPage, tMyList* pLinkList)
{
    int retVal;
    regmatch_t regRes;
    tMyString link;

    retVal = rxmatch(pPage, &regRes, urlPattern);
    if(retVal == 0)
    {
        link.string = &pPage[regRes.rm_so];
        link.size = regRes.rm_eo - regRes.rm_so;
        putLinkIntoList(pLinkList, &link);
        processPage(&pPage[regRes.rm_eo], pLinkList);
    }
    else
    {
        return;
    }
}

static void putLinkIntoList(tMyList *pList, tMyString *pLink)
{
    pList->listOfLinks = (tMyString* )realloc(pList->listOfLinks, sizeof(tMyString) * (pList->numberOfLinks + 1u));
    if(pList->listOfLinks == NULL)
    {
        printf("Out of memory!\n");
    }
    pList->listOfLinks[pList->numberOfLinks].string = (char*)malloc(sizeof(char) * pLink->size + 1u);
    if(pList->listOfLinks[pList->numberOfLinks].string == NULL)
    {
        printf("Out of memory!\n");
    }
    memcpy(pList->listOfLinks[pList->numberOfLinks].string, pLink->string, pLink->size + 1);
    pList->listOfLinks[pList->numberOfLinks].string[pLink->size] = '\0';
    pList->listOfLinks[pList->numberOfLinks].size = pLink->size;
    pList->numberOfLinks += 1u;
}


static tMyString getLinkFromList(tMyList *pList, const int idx)
{
    tMyString retVal;

    if(pList != NULL)
    {
        retVal = pList->listOfLinks[idx];
    }

    return retVal;
}

static void deleteList(tMyList *pList)
{
    free(pList->listOfLinks);
}

static void deleteBuffer(tMyString *pString)
{
    free(pString->string);
}

static size_t writeToBuffer(void *pContents, size_t pSize, size_t pNmemb, void *pUserp)
{
    size_t currentSize = pSize * pNmemb;
    tMyString *currentData = (tMyString *)pUserp;
    currentData->string = (char*)realloc(currentData->string, sizeof(char) * (currentData->size + currentSize + 1u));
    if(currentData->string == NULL)
    {
        printf("Out of memory!/n");
    }
    memcpy(&(currentData->string[currentData->size]), pContents, currentSize);
    currentData->size += currentSize;
    currentData->string[currentData->size] = '\0';
    return currentSize;
}

static int rxmatch(const char *pString, regmatch_t *result, const char *pPattern)
{
    regex_t rx;
    regmatch_t res;
    int retVal = 0;
    regcomp(&rx, pPattern, REG_EXTENDED);
    retVal = regexec(&rx, pString, 1u, &res, 0u);
    regfree(&rx);
    *result = res;
    return retVal;
}

/* Example program to show how curl can be used. */
int main(int argc, char **argv)
{
    int deep = 0u;
    int idx  = 0u;
    tThreadData startData;
    tMyString startURL;

    if(3 != argc)
    {
        printf("Usage: %s <url>\n, %s <deep> ", argv[1], argv[2]);
    }
    else
    {
        startURL.string = argv[1];
        startURL.size = strlen(argv[1]);
        deep = atoi(argv[2]);

        startData.link = &startURL;
        startData.deep = deep;

        g_listOfLinks.numberOfLinks = 0u;
        g_listOfLinks.listOfLinks = (tMyString*)malloc( 1u * sizeof(tMyString) );

        webcrawler((void*)&startData);

        for(idx = 0u; idx < g_listOfLinks.numberOfLinks; idx++)
        {
            printf("%d. is: %s\n", idx, g_listOfLinks.listOfLinks[idx].string);
        }
    }

    deleteList(&g_listOfLinks);

    /* Last thing that main() should do */
    pthread_exit(NULL);
}
/* End of the example program. */
