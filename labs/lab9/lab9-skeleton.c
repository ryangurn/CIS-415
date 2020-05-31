/*=============================================================================
 * Program Name: lab8
 * Author: Jared Hall, <your name>
 * Date: <date>
 * Description:
 *     A simple program that implements a thread-safe queue of meal tickets
 *
 * Notes:
 *     1. DO NOT COPY-PASTE MY CODE INTO YOUR PROJECTS.
 *===========================================================================*/

//========================== Preprocessor Directives ==========================
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
//=============================================================================

//================================= Constants =================================
#define MAXNAME 15
#define MAXQUEUES 4
#define MAXTICKETS 3
#define MAXDISH 20
#define MAXPUBS 3
#define MAXSUBS 4
//=============================================================================

//============================ Structs and Macros =============================
//TODO: Add a mutex init
//TODO: Declare a mutex for each queue (HINT: I placed my mutexes in the MTQ/TEQ)
int TICKET = 0;

typedef struct mealTicket{
	int ticketNum;
	char *dish;
} mealTicket;

typedef struct MTQ {
	char name[MAXNAME];
	int head;
	int tail;
	int length;
	int ticket;
	mealTicket *buffer;
	pthread_mutex_t mutex;
} MTQ;

MTQ registry[MAXQUEUES];
mealTicket buffer[MAXQUEUES][MAXTICKETS+1];

struct enqueueArgs
{
	char *name;
	mealTicket *TE;
	int thread;
};

struct dequeueArgs
{
	char *name;
	mealTicket MT;
	int ticketNum;
	int thread;
};

struct enqueueArgs eArgs[MAXPUBS];
struct dequeueArgs dArgs[MAXSUBS];

pthread_t pubs[MAXPUBS];
pthread_t subs[MAXSUBS];

mealTicket nullTicket = {-1};

void init(int pos, char *MTQ_ID, int len) {
	strcpy(registry[pos].name, MTQ_ID);
	registry[pos].buffer = (mealTicket *)malloc(sizeof(mealTicket)*(len+1));
	registry[pos].head = 0;
	registry[pos].tail = 0;
	registry[pos].length = len;
	registry[pos].ticket = 0;

	pthread_mutex_init(&registry[pos].mutex, NULL);
	registry[pos].ticket = 0;
	//add null ticket or use head-1 to split empty/full case. Up to you.
	registry[pos].buffer[registry[pos].length] = nullTicket;
}

void freeMTQ(int pos, char *MTQ_ID) {
	free(registry[pos].buffer);
}

//=============================================================================

//================================= Functions =================================
int enqueue(char *MTQ_ID, mealTicket *MT) {
	int ret = 0;
	int i, flag = 0;
	//Step-1: Find registry
	for(i=0;i<MAXQUEUES;i++) {
		if(strcmp(MTQ_ID, registry[i].name) == 0) { flag = 1; break; }
	}

	//STEP-2: Enqueue the ticket
	if(flag) {
		pthread_mutex_lock(&registry[i].mutex);

		int tail = registry[i].tail;
		if(registry[i].buffer[tail].ticketNum != -1) {
			// registry[i].buffer[tail].ticketNum = registry[i].ticket;
			// registry[i].buffer[tail].dish =  MT->dish;
			// registry[i].ticket++;
			MT->ticketNum = TICKET;
			TICKET++;
			registry[i].buffer[tail] = *MT;
			if(tail == registry[i].length) { registry[i].tail = 0; }
			else { registry[i].tail++; }
			ret = 1;
		}

		pthread_mutex_unlock(&registry[i].mutex);
	}
	return ret;
}

int dequeue(char *MTQ_ID, int ticketNum, mealTicket *MT) {
	int ret = 0;
	int i, flag = 0;

	//Step-1: Find registry
	for(i=0;i<MAXQUEUES;i++) {
		if(strcmp(MTQ_ID, registry[i].name) == 0) { flag = 1; break; }
	}

	//Step-2: Dequeue the ticket
	if(flag) {
		pthread_mutex_lock(&registry[i].mutex);

		int head = registry[i].head;
		int tail = registry[i].tail;

		if(head != tail && ticketNum == registry[i].buffer[head].ticketNum) {
			//copy the ticket
			MT->ticketNum = registry[i].buffer[head].ticketNum;
			MT->dish = registry[i].buffer[head].dish;

			//change the null ticket to empty
			if(head == 0) {
				registry[i].buffer[registry[i].length].ticketNum = 0;
			} else {
				registry[i].buffer[head-1].ticketNum = 0;
			}

			//change the current ticket to null
			registry[i].buffer[head].ticketNum = -1;

			//increment the head
			if(head == registry[i].length+1) { registry[i].head = 0; }
			else { registry[i].head++; }
			ret = 1;
		}

		pthread_mutex_unlock(&registry[i].mutex);
	}
	return ret;
}

void *publisher(void *args) {
	/* TODO: The publisher will receive the following in the struct args:
	*        1. An array of mealTickets to push to the queue.
	*        2. For each meal ticket, an MTQ_ID. (Note: a publisher can push to multiple queues)
	*        3. The thread ID
	*        4. The threads state: alive=1|dead=0
	* The publisher will then print its type and thread ID on startup. Then it
	* will aquire a lock in before calling the enqueue method to push a meal ticket to
	* its appropriate queue before sleeping for 1 second.
	* It will do this until there are no more meal tickets to push.
	*/
	char *name;
	int j, thread;
	name = ((struct enqueueArgs *) args)->name;
	thread = ((struct enqueueArgs *) args)->thread;

	printf("Pub - Queue %s (ThreadID: %d)\n", name, thread);
	for (j = 0; j < 3; j++)
	{
		mealTicket MT;
		MT = ((struct enqueueArgs *) args)->TE[j];
		enqueue(name, &MT);
	}
}

void *subscriber(void *args) {
	/* TODO:The subscriber will take the following:
	* 1. The MTQ_ID's it will pull from. (Note: A subscriber can pull from multiple queues.)
	* 2. The thread ID
	* 3. A temp meal ticket struct.
	* 4. The threads state: alive=1|dead=0

	* The subscriber will print its type and thread ID on startup. Then it will pull a ticket from its queue
	* and print it. If the queue is empty then it will print an empty message along with its
	* thread ID and wait for 1 second. If the thread is not empty then it will pop a ticket and
	* print it along with the thread id.
	*/
	char *name;
	int thread, ticket;
	mealTicket MT;

	MT = ((struct dequeueArgs *) args)->MT;
	name = ((struct dequeueArgs *) args)->name;
	thread = ((struct dequeueArgs *) args)->thread;
	ticket = ((struct dequeueArgs *) args)->ticketNum;

	printf("Sub - Queue %s (ThreadID: %d)\n", name, thread);

	dequeue(name, ticket, &MT);
	printf("Ticket: %d -> Dish(%s)\n", MT.ticketNum, MT.dish);
}
//=============================================================================

//=============================== Program Main ================================
int main(int argc, char argv[]) {
	//Variables Declarations
	char *qNames[] = {"Breakfast", "Lunch", "Dinner", "Bar"};
	char *bFood[] = {"Eggs", "Bacon", "Steak"};
	char *lFood[] = {"Burger", "Fries", "Coke"};
	char *dFood[] = {"Steak", "Greens", "Pie"};
	char *brFood[] = {"Whiskey", "Sake", "Wine"};
	int i, j, t = 1;
	int test[4];
	char dsh[] = "Empty";
	mealTicket bfast[3] = {[0].dish = bFood[0], [1].dish = bFood[1], [2].dish = bFood[2]};
	mealTicket lnch[3] = {[0].dish = lFood[0], [1].dish = lFood[1], [2].dish = lFood[2]};
	mealTicket dnr[3] = {[0].dish = dFood[0], [1].dish = dFood[1], [2].dish = dFood[2]};
	mealTicket br[3] = {[0].dish = brFood[0], [1].dish = brFood[1], [2].dish = brFood[2]};
	mealTicket ticket = {.ticketNum=0, .dish=dsh};

	//STEP-1: Initialize the registry
	for(int i=0; i<4; i++) {
		init(i, qNames[i], 3);
	}

	//STEP-2: Create the publisher thread-pool

	eArgs[0].name = qNames[0];
	eArgs[0].TE = bfast;
	eArgs[0].thread = 010;

	pthread_create(&pubs[0], NULL, publisher, (void *)&eArgs[0]);

	eArgs[1].name = qNames[1];
	eArgs[1].TE = lnch;
	eArgs[1].thread = 110;

	pthread_create(&pubs[1], NULL, publisher, (void *)&eArgs[1]);

	eArgs[2].name = qNames[2];
	eArgs[2].TE = dnr;
	eArgs[2].thread = 111;

	pthread_create(&pubs[2], NULL, publisher, (void *)&eArgs[2]);

	eArgs[3].name = qNames[3];
	eArgs[3].TE = br;
	eArgs[3].thread = 000;

	pthread_create(&pubs[3], NULL, publisher, (void *)&eArgs[3]);
	sleep(1);

	//STEP-3: Create the subscriber thread-pool

	for (i = 0; i < 4; i++)
	{
		mealTicket MT;
		dArgs[i].name = qNames[i];
		dArgs[i].ticketNum = registry[i].buffer[registry[i].head].ticketNum;
		dArgs[i].MT = MT;
		dArgs[i].thread = i+20;

		pthread_create(&subs[i], NULL, subscriber, (void *)&dArgs[i]);
	}

	//STEP-4: Join the thread-pools
	for (i=0; i <MAXPUBS; i++)
	{
		pthread_join(pubs[i], NULL);
		pthread_join(subs[i], NULL);
	}

	//STEP-5: Free the registry
	for (size_t i = 0; i < 4; i++) {
		freeMTQ(i, ""); // the string does not appear to be used for this
	}
	return EXIT_SUCCESS;
}
//=============================================================================
