#define  _GNU_SOURCE

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>

#define TRUE 1
#define FALSE 0
#define DEBUG FALSE
#define MAXENTRIES 5
#define URLSIZE 256
#define CAPSIZE 256
#define NUMPROXIES 10
#define MAXNAME 256
#define MAXTOPICS 5

char *left_trim (char *s)
{
	while ( isspace(*s)) s ++;
	return s;
}

char *right_trim (char *s)
{
	char *back = s + strlen (s);
	while ( isspace(*-- back));
	*( back + 1 ) = '\0';
	return s;
}

char *trim (char *s)
{ return right_trim (left_trim (s)); }

typedef struct
{
	int entryNum;                              // entry number
	struct timeval timeStamp;                  // created time
	pthread_t pubID;                           // publisher id
	char photoURL[URLSIZE];                    // url of topic
	char photoCaption[CAPSIZE];                // photo caption
} topicEntry;

typedef struct
{
	int head, tail, length, counter, topicID; // head, tail, length, counter &
	// topic for various purposes
	topicEntry *buffer;                       // buffer for storing topicEntry
	char name[MAXNAME];                      // name of the queue
	pthread_mutex_t primaryMutex;             // the main mutex
	pthread_mutex_t secondaryMutex;           // secondary mutex
} Buffer;

typedef struct
{
	pthread_t ID;                             // the thread id
	int flag;                                 // flag to change state
	char location[MAXNAME];                   // name of thread
} threadArgs;


int numTopics = 0;          // the number of total topics
int positionTopic = 0;      // position to init
float Delta = 0;              // the delta

Buffer Queue[MAXTOPICS];    // registry that will hold kinda everything
// thread arguements for publishers and subscribers
threadArgs publisherArgs[NUMPROXIES];
threadArgs subscriberArgs[NUMPROXIES];
// null and empty entries for init
topicEntry nullTopic = {.entryNum = - 1};
topicEntry emptyTopic = {.entryNum = 0};

// initialize mutex and thread conditions
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t conditions = PTHREAD_COND_INITIALIZER;

/*
 Enqueue(int id, topicEntry topic)
 id - the id of the queue that we are insertting the topic entry
 topic - the topicEntry data that is being enqueued
*/
int enqueue (int id, topicEntry *topic)
{
	for ( int i = 0; i < numTopics; i ++ )
	{
		// find the topic buffer
		if ( id == Queue[i].topicID )
		{

			pthread_mutex_lock (&Queue[i].primaryMutex); // lock primary
			// if we cannot get the secondary lock then unlock the primary & attempt to get
			// the secondary mutex. This ensures that we are getting things locked correctly
			while ( pthread_mutex_trylock (&Queue[i].secondaryMutex) != 0 )
			{
				pthread_mutex_unlock (&Queue[i].primaryMutex); // unlock primary
				sched_yield ();
				pthread_mutex_lock (&Queue[i].primaryMutex); // lock seoncdary
			} // while()

			// if the queue is full
			if ( Queue[i].buffer[Queue[i].tail].entryNum == - 1 )
			{
				if ( DEBUG )
				{
					printf ("The queue(%d) is full\n", id);
					fflush(stdout);
				}

				// unlock both primary and secondary mutex
				pthread_mutex_unlock (&Queue[i].primaryMutex);
				pthread_mutex_unlock (&Queue[i].secondaryMutex);

				return 0; // stop
			} // if()

			// set the entry and timestamp
			topic->entryNum = Queue[i].counter;
			gettimeofday (&topic->timeStamp, NULL);

			// move counter/tail forward, set the the buffer
			Queue[i].counter = Queue[i].counter + 1;
			Queue[i].buffer[Queue[i].tail] = *topic;
			Queue[i].tail = Queue[i].tail + 1;

			// check if the tail is at the end
			if ( Queue[i].tail == Queue[i].length )
			{
				Queue[i].tail = 0;
			} // if()

			if ( DEBUG )
			{
				printf ("Enqueued!\n");
				fflush(stdout);
			}

			// unlock both mutexs
			pthread_mutex_unlock (&Queue[i].primaryMutex);
			pthread_mutex_unlock (&Queue[i].secondaryMutex);

			// return 1;
			return 1;

		} // if()

	} // for()

	if ( DEBUG )
	{
		printf ("The queue(%d) was not found\n", id);
		fflush(stdout);
	}
	return 0;

} // enqueue()

/*
Dequeue(null)
*/
int dequeue ()
{
	// setup current time for comparison
	struct timeval curr;
	gettimeofday (&curr, NULL);
	for ( int i = 0; i < numTopics; i ++ )
	{
		// lock the primary mutex
		pthread_mutex_lock (&Queue[i].primaryMutex);
		// if we cannot get the secondary lock then unlock the primary & attempt to get
		// the secondary mutex. This ensures that we are getting things locked correctly
		while ( pthread_mutex_trylock (&Queue[i].secondaryMutex) != 0 )
		{
			pthread_mutex_unlock (&Queue[i].primaryMutex); // unlock primary
			sched_yield ();
			pthread_mutex_lock (&Queue[i].primaryMutex); // lock seoncdary
		} // while()

		// loop through the buffer itself
		for ( int j = 0; j < Queue[i].length;
		      j ++ ) //TODO: DETERMINE IF THIS IS NEEDED (I DONT THINK SO BUT I WILL CHECK)
		{
			if ( Queue[i].buffer[Queue[i].head].entryNum > 0 )
			{
				// have we been waiting too long
				if ( curr.tv_sec - Queue[i].buffer[Queue[i].head].timeStamp.tv_sec > Delta )
				{

					Queue[i].buffer[Queue[i].head] = nullTopic;

					// check if the head is at the start
					if ( Queue[i].head == 0 )
					{
						Queue[i].buffer[Queue[i].length - 1] = emptyTopic; // set the last item to the empty topic
					}
					else // otherwise use the head
					{
						Queue[i].buffer[Queue[i].head - 1] = emptyTopic; // set the last item to the empty topic
					} // if()

					// iterate the head;
					Queue[i].head = Queue[i].head + 1;

					// if the head is at the end
					if ( Queue[i].head == Queue[i].length )
					{
						Queue[i].head = 0; // reset to start
					} // if()

					printf ("Dequeued!\n");
					fflush(stdout);

				} // if()

			} // if()
		} // for()

		// unlock
		pthread_mutex_unlock (&Queue[i].primaryMutex);
		pthread_mutex_unlock (&Queue[i].secondaryMutex);

	} // for()

	return 0;
} // dequeue()

/*
entry(int id, int last, topicEntry *topic)
id - The id of the queue that we are looking for
last - the last entry in the queue
topic - the topic that we are attempting to find
*/
int entry (int id, int last, topicEntry *topic)
{
	// loop through topic buffers
	for ( int i = 0; i < numTopics; i ++ )
	{
		// can we identify the correct queue
		if ( id == Queue[i].topicID )
		{

			// lock the primary mutex
			pthread_mutex_lock (&Queue[i].primaryMutex);
			// if we cannot get the secondary lock then unlock the primary & attempt to get
			// the secondary mutex. This ensures that we are getting things locked correctly
			while ( pthread_mutex_trylock (&Queue[i].secondaryMutex) != 0 )
			{
				pthread_mutex_unlock (&Queue[i].primaryMutex); // unlock primary
				sched_yield ();
				pthread_mutex_lock (&Queue[i].primaryMutex); // lock seoncdary
			} // while()

			// check for empty Queue
			if ( Queue[i].head == Queue[i].tail )
			{

				if ( DEBUG )
				{
					printf ("The queue(%d) is empty\n", id);
					fflush(stdout);
				}

				// unlock the mutex's
				pthread_mutex_unlock (&Queue[i].primaryMutex);
				pthread_mutex_unlock (&Queue[i].secondaryMutex);

				//stop
				return 0;

				int j = Queue[i].head;
				// loop until we are done
				while ( j != - 1 )
				{

					if ( Queue[i].buffer[j].entryNum >= last + 1 )
					{
						*topic = Queue[i].buffer[j];
						if ( Queue[i].buffer[j].entryNum > last + 1 )
						{
							pthread_mutex_unlock (&Queue[i].primaryMutex);
							pthread_mutex_unlock (&Queue[i].secondaryMutex);
							return Queue[i].buffer[j].entryNum;
						} // if()

						pthread_mutex_unlock (&Queue[i].primaryMutex);
						pthread_mutex_unlock (&Queue[i].secondaryMutex);
						return 1;
					} // if()

					if ( j == Queue[i].length - 1 )
					{
						if ( Queue[i].head == 0 )
						{
							pthread_mutex_unlock (&Queue[i].primaryMutex);
							pthread_mutex_unlock (&Queue[i].secondaryMutex);
							return 0;
						} // if()
						j = 0;
					} // if()

					if ( j == Queue[i].head - 1 )
					{

						pthread_mutex_unlock (&Queue[i].primaryMutex);
						pthread_mutex_unlock (&Queue[i].secondaryMutex);
						return 0;

					} // if()
					j = j + 1;

				} // while()

				pthread_mutex_unlock (&Queue[i].primaryMutex);
				pthread_mutex_unlock (&Queue[i].secondaryMutex);
				return 0;

			} // if()

		} // if()
	} // for()
	if ( DEBUG )
	{
		printf ("The queue(%d) was not found\n", id);
		fflush(stdout);
	}
	return 0;
} // entry()

/*
position(pthread_t id, int flag)
id - the thread that we are attempting to find in either the publisher or subscriber thread pools
flag - the way to denote if we are looking for sub = 1, or pub = 0
*/
int position (pthread_t id, int flag)
{
	for ( int i = 0; i < NUMPROXIES; i ++ )
	{
		if ( flag == 0 )
		{
			if ( id == publisherArgs[i].ID )
			{
				return i;
			} // if()
		} // if()

		if ( flag == 1 )
		{
			if ( id == subscriberArgs[i].ID )
			{
				return i;
			}
		} //if()
	} // for()
	return - 1;
} // position()

/*
Publisher()
*/
void *Publisher (void *args)
{
	printf ("Proxy thread %ld - Type(Publisher)\n", pthread_self ());
	fflush(stdout);


	int pubPos = - 1;
	int processing = 1;
	while ( processing == 1 )
	{
		// lock the main mutex
		pthread_mutex_lock (&mutex);
		pthread_cond_wait (&conditions, &mutex);
		pthread_mutex_unlock (&mutex);

		pubPos = position (pthread_self (), 0);
		if ( pubPos != - 1 && publisherArgs[pubPos].flag == 1 )
		{
			FILE *pubFile = fopen (publisherArgs[pubPos].location, "r");

			if ( pubFile == NULL )
			{
				printf ("The publisher file(%s) was not able to be opened!\n", publisherArgs[pubPos].location);
				fflush(stdout);
				return 0;
			} // if()

			int pubNumber = 1;
			while ( pubNumber != -1)
			{

				char cmd[32];
				char *line = NULL;
				size_t length = 0;
				pubNumber = getline (&line, &length, pubFile);
				sscanf (line, "%15s", cmd);

				// printf ("--> cmd(%s)\n\n", cmd);
				// fflush (stdout);

				if ( strcmp (trim(cmd), "put") == 0 )
				{
					int id;
					char url[URLSIZE];
					char caption[CAPSIZE];

					sscanf (line, "put %d \"%254[^\"]\" \"%254[^\"]\"", &id, url, caption);

					if ( id <= 0 || strcmp (url, "") == 0 || strcmp (caption, "") == 0 )
					{
						printf ("Invalid arguments for put command. Thread(%ld)\n", pthread_self ());
						fflush(stdout);
						processing = 0;
					} // if()

					printf ("Proxy thread %ld - Type:(Publisher) - Executed command: put\n", pthread_self ());
					fflush(stdout);
					topicEntry pub = {.pubID = pthread_self ()};
					strcpy (pub.photoURL, url);
					strcpy (pub.photoCaption, caption);

					topicEntry *pubPtr = &pub;
					enqueue (id, pubPtr);

				} // if()
				else if ( strcmp (trim(cmd), "sleep") == 0 )
				{
					int time;
					sscanf (line, "sleep %d", &time);

					if ( time >= 0 )
					{
						printf ("Proxy thread %ld - Type:(Publisher) - Executed command: sleep\n", pthread_self ());
						fflush(stdout);
						usleep (time);
					} // if()
					else
					{
						printf ("Invalid arguments for sleep command. Thread(%ld)", pthread_self ());
						fflush(stdout);
					} // else()
				} // if()

				else if ( strcmp (trim(cmd), "stop") == 0 )
				{
					printf ("Proxy thread %ld - Type:(Publisher) - Executed command: stop\n", pthread_self ());
					fflush(stdout);
					publisherArgs[pubPos].flag = 0;
				} // if()
				free (line);
			} // while()

			fclose (pubFile);

		} // if()
		processing = 0;

	} // while()

	return 0;
} // publisher()

/*

*/
void *Subscriber (void *args)
{
	printf ("Proxy thread %ld - Type(Subscriber)\n", pthread_self ());
	fflush(stdout);

	int pos = - 1;
	int processing = 1;
	int lastEntry[MAXTOPICS];
	int entryReturn = 1;

	for ( int i = 0; i < MAXTOPICS; i ++ )
	{
		lastEntry[i] = 1;
	} // for()

	while ( processing )
	{
		// lock the main mutex
		pthread_mutex_lock (&mutex);
		pthread_cond_wait (&conditions, &mutex);
		pthread_mutex_unlock (&mutex);
		pos = position (pthread_self (), 1);
		if ( pos != - 1 && subscriberArgs[pos].flag == 1 )
		{
			char *token = NULL;
			char location[MAXNAME];

			strcpy (location, subscriberArgs[pos].location);

			// declare vars for determining the new file name
			char *ext = ".html";
			char *ptr;
			token = strtok (location, ".");


			// setup the correct file name
			if ( token != NULL)
			{
				strncat (token, ext, 6);
			} // if()


			printf("-----> %s \n\n", token);
			fflush(stdout);

			// open the file to write to
			FILE *subFile = fopen (token, "w");
			if ( subFile == NULL)
			{
				printf ("File(%s) does not exist and thus cannot be modified\n", token);
				fflush(stdout);
				return;
			} // if()
			else
			{
				fprintf (subFile, "<!DOCTYPE html>");
				fprintf (subFile, "<html>");
				fprintf (subFile, "<head>");
				fprintf (subFile, "<title>%s</title>\n\n", token);
				fprintf (subFile, "<style>\n");
				fprintf (subFile, "table, th, td {\n");
				fprintf (subFile, "\tborder: 1px solid black;\n");
				fprintf (subFile, "\tborder-collapse: collapse;\n");
				fprintf (subFile, "}\n");
				fprintf (subFile, "th, td {\n");
				fprintf (subFile, "\tpadding: 5px;\n");
				fprintf (subFile, "}\t");
				fprintf (subFile, "th {\n");
				fprintf (subFile, "\ttext-align:left;\n");
				fprintf (subFile, "}\n\n");
				fprintf (subFile, "</style>\n\n");
				fprintf (subFile, "</head>");
				fprintf (subFile, "<body>\n\n");
				fprintf (subFile, "<h1>Subscriber: %s</h1>", token);
			}

			// open the input subscriber file
			FILE *in = fopen (subscriberArgs[pos].location, "r");

			// check for errors
			if ( in == NULL)
			{
				printf ("File(%s) could not be opened\n", subscriberArgs[pos].location);
				fflush(stdout);
				return 0;
			} // if()

			int inNumber = 1;
			while ( inNumber != -1)
			{

				char cmd[32];
				char *line = NULL;
				size_t length = 0;
				inNumber = getline (&line, &length, in);

				int j = 0;
				char *token;
				char *save_ptr;
				char str[100];
				char *strArr[1000];

				token = strtok_r (line, " ", &save_ptr);
				while ( token != NULL)
				{
					if ( token[0] == '"' )
					{
						int i;
						int iter = 0;
						for ( i = 1; token[i] != '"'; i ++ )
						{
							str[iter] = token[i];
							iter ++;
						} // for()
						strArr[j] = str;
					} // if()
					else
					{
						strArr[j] = token;
					} // else()

					j ++;
					token = strtok_r (NULL, " ", &save_ptr);
				} // while()

				if ( strcmp (strArr[0], "get") == 0 )
				{
					printf ("Proxy Thread: %ld - type: Subscriber - Executed command : get\n", pthread_self ());
					fflush(stdout);
					topicEntry ent;
					topicEntry *entPtr = &ent;
					int id = atoi (strArr[1]);

					entryReturn = entry (id, lastEntry[id], entPtr);

					if ( entryReturn == 1 )
					{
						lastEntry[id] = lastEntry[id] + 1;
						printf ("URL(%s) | Caption(%s) | ID(%ld)", ent.photoURL, ent.photoCaption, ent.pubID);
						fflush(stdout);

						int rPos = 0;
						for ( int l = 0; l < MAXTOPICS; l ++ )
						{
							if ( id == Queue[l].topicID )
							{
								rPos = l;
							} // if()
						} // for()

						fprintf (subFile, "\n<h2>Topic Name: %s</h2>\n", Queue[rPos].name);
						fprintf (subFile,
						         "<table style='width:100%%\' \align='middle'>\n\t<tr>\n\t\t<th>CAPTION</th>\n\t\t<th>PHOTO-URL</th>\n\t</tr>\n");
						fprintf (subFile, "\t<tr>");
						fprintf (subFile, "\t\t<td>%s</td>\n", ent.photoCaption);
						fprintf (subFile, "\t\t<td>%s</td>\n", ent.photoURL);
						fprintf (subFile, "\t</tr>\n");
						fprintf (subFile, "</table>\n");


					} // if()

					if ( entryReturn > 1 )
					{
						lastEntry[id] = entryReturn;
						printf ("URL(%s) | Caption(%s) | ID(%ld)", ent.photoURL, ent.photoCaption, ent.pubID);
						fflush(stdout);

						int rPos = 0;
						for ( int f = 0; f < MAXTOPICS; f ++ )
						{

							if ( id == Queue[f].topicID )
							{
								rPos = f;
							} // id()
						} // for()

						fprintf (subFile, "\n<h2>Topic Name: %s</h2>\n", Queue[rPos].name);
						fprintf (subFile,
						         "<table style='width:100%%\' \align='middle'>\n\t<tr>\n\t\t<th>CAPTION</th>\n\t\t<th>PHOTO-URL</th>\n\t</tr>\n");
						fprintf (subFile, "\t<tr>");
						fprintf (subFile, "\t\t<td>%s</td>\n", ent.photoCaption);
						fprintf (subFile, "\t\t<td>%s</td>\n", ent.photoURL);
						fprintf (subFile, "\t</tr>\n");
						fprintf (subFile, "</table>\n");

					} // if()

				} // if()

				else if ( strcmp (strArr[0], "sleep") == 0 )
				{
					printf ("Proxy Thread: %ld - type: Subscriber - Executed command : sleep\n", pthread_self ());
					fflush(stdout);
					usleep (atoi (strArr[1]));
				} // if()

				else if ( strcmp (strArr[0], "stop") == 0 )
				{
					printf ("Proxy Thread : %ld - Type: Subscriber - executed command : stop\n", pthread_self ());
					fflush(stdout);
					subscriberArgs[pos].flag = 0;
				} // if()

				fprintf (subFile, "\n</body>\n");
				fprintf (subFile, "</html>\n");

				free (line);
			} // while()

			fclose (in);
			fclose (subFile);

		} // if()

		processing = 0;

	} // while()
	return 0;
} // subscriber()

/*

*/
void *Clean (void *args)
{
	printf ("Clean Thread Running thread(%ld), running now\n", pthread_self ());
	fflush(stdout);

	while ( TRUE )
	{
		// wait (5);
		dequeue ();

		int freeCount = 0;
		for ( int m = 0; m < NUMPROXIES; m ++ )
		{
			if ( subscriberArgs[m].flag == 0 )
			{
				freeCount = freeCount + 1;
			} // if
			else
			{
				printf("waiting on subscriber(%ld)\n", subscriberArgs[m].ID);
				fflush(stdout);
			}


			if ( publisherArgs[m].flag == 0 )
			{
				freeCount = freeCount + 1;
			} // if
			else
			{
				printf("waiting on publisher(%ld)\n", publisherArgs[m].ID);
				fflush(stdout);
			}
		} // for()

		printf("NUM(%d)\n", NUMPROXIES * 2);
		printf("free(%d)\n", freeCount);
		fflush(stdout);

		if ( freeCount == NUMPROXIES * 2 )
		{
			printf ("Clean thread stopping\n");
			fflush(stdout);
			return;
		}

	} // while()
} // clean()


/*
initBuffer (int len, int id, char *name)
len - length of the buffer
id - topic id for the buffer
name - buffer name
*/
void initBuffer (int len, int id, char *name)
{
	topicEntry buf[len + 1];

	for ( int s = 0; s < len; s ++ )
	{
		buf[s] = emptyTopic;
	} // for()

	buf[len] = nullTopic;

	Buffer buffer = {.buffer = buf, .head = 0, .tail = 0, .length = len +
	                                                                1, .counter = 1, .primaryMutex = PTHREAD_MUTEX_INITIALIZER, .secondaryMutex = PTHREAD_MUTEX_INITIALIZER, .topicID = id};
	strcpy (buffer.name, name);

	if ( numTopics == MAXTOPICS )
	{
		Queue[positionTopic] = buffer;
		positionTopic ++;

		if ( positionTopic == MAXTOPICS )
		{
			positionTopic = 0;
		}

	} // if()
	else
	{
		Queue[numTopics] = buffer;
		numTopics ++;
	} // else

} // initBuffer()

void Start ()
{
	pthread_cond_broadcast (&conditions);
}

/*

*/
int main (int argc, char *argv[])
{
	if ( argv[1] == NULL || argv[2] != NULL)
	{
		printf ("Error: invalid args\n");
		fflush(stdout);
		exit (1);
	}

	FILE *file = fopen (argv[1], "r");
	if ( file == NULL)
	{
		printf ("Error: failed to open file.\n");
		fflush(stdout);
		exit (1);
	}

	pthread_t threads[NUMPROXIES * 2];
	pthread_t cleanup;

	for ( int i = 0; i < NUMPROXIES; i ++ )
	{
		pthread_create (&threads[i], NULL, &Publisher, NULL);
		publisherArgs[i].ID = threads[i];
		publisherArgs[i].flag = 0;

		pthread_create (&threads[i + NUMPROXIES], NULL, &Subscriber, NULL);
		subscriberArgs[i].ID = threads[i + NUMPROXIES];
		subscriberArgs[i].flag = 0;
	}

	int number = 1;
	while ( number != -1)
	{
		size_t length = 0;
		char *line = NULL;
		char cmd[32];
		number = getline (&line, &length, file);
		sscanf (line, "%15s", cmd);

		// if ( DEBUG )
		// {
		// 	printf ("--> %s\n", line);
		// 	printf ("---> %s\n", cmd);
		// 	fflush (stdout);
		// }

		if ( strcmp (cmd, "create") == 0 )
		{
			int topicID = 0;
			int queueLength;
			char name[256] = "";

			sscanf (line, "create topic %d \"%127[^\"]\" %d", &topicID, name, &queueLength);
			if ( topicID <= 0 || queueLength <= 0 || strcmp (name, "") == 0 )
			{
				printf ("Error: could not create topic, null information\n");
				fflush(stdout);
			} // if()
			else
			{
				initBuffer (length, topicID, name);
				printf ("Topic Created!\n");
				fflush(stdout);
			} // else
		} // if()
		else if ( strcmp (cmd, "query") == 0 )
		{
			char variable[128];
			int k;
			sscanf (line, "query %63s", variable);

			if ( strcmp (variable, "publishers") == 0 )
			{
				for ( int i = 0; i < NUMPROXIES; i ++ )
				{
					if ( publisherArgs[i].flag == 1 )
					{
						printf ("Publisher Thread(%ld), Location(%s)\n", publisherArgs[i].ID,
						        publisherArgs[i].location);
										fflush(stdout);
					}
				} // for()
			} // if()
			else if ( strcmp (variable, "subscribers") == 0 )
			{
				for ( int i = 0; i < NUMPROXIES; i ++ )
				{
					if ( subscriberArgs[i].flag == 1 )
					{
						printf ("Subscriber Thread(%ld), Location(%s)\n", subscriberArgs[i].ID,
						        subscriberArgs[i].location);
										fflush(stdout);
					}
				} // for()
			} // else if()
			else if ( strcmp (variable, "topics") == 0 )
			{
				for ( int i = 0; i < numTopics; i ++ )
				{
					printf ("ID(%d), Len(%d)\n", Queue[i].topicID, Queue[i].length);
					fflush(stdout);
				} //for()
			}
		}// else if()
		else if ( strcmp (cmd, "add") == 0 )
		{
			char variable[128];
			char filename[128] = "";

			sscanf (line, "add %63s \"%127[^\"]\"", variable, filename);

			if ( strcmp (variable, "publisher") == 0 )
			{
				for ( int i = 0; i < NUMPROXIES; i ++ )
				{
					if ( publisherArgs[i].flag == 0 )
					{
						publisherArgs[i].flag = 1;
						strcpy (publisherArgs[i].location, filename);
						i = NUMPROXIES;
						printf ("Added publisher\n");
						fflush(stdout);
					} // if()

					if ( i == NUMPROXIES - 1 )
					{
						printf ("No free threads in the pool of publishers\n");
						fflush(stdout);
					} // if()
				} // for()
			} // if()
			else if ( strcmp (variable, "subscriber") == 0 )
			{
				for ( int i = 0; i < NUMPROXIES; i ++ )
				{
					if ( subscriberArgs[i].flag == 0 )
					{
						subscriberArgs[i].flag = 1;
						strcpy (subscriberArgs[i].location, filename);
						i = NUMPROXIES;
						printf ("Added subscriber\n");
						fflush(stdout);
					} // if()

					if ( i == NUMPROXIES - 1 )
					{
						printf ("No free threads in the pool of subscribers\n");
						fflush(stdout);
					} // if()
				} // for()
			} // else if()
			else
			{
				printf ("Error: unrecognized item to append: %s\n", variable);
			} // else()
		} // else if()
		else if ( strcmp (cmd, "delta") == 0 )
		{
			float de = 0;
			sscanf (line, "delta %f", &de);
			if ( de > 0 )
			{
				printf ("Delta = %f\n", de);
				fflush(stdout);
				Delta = de;
			} // if()
			else
			{
				printf ("Error: invalid delta\n");
				fflush(stdout);
			} // else()
		} // else if()
		else if ( strcmp (cmd, "start") == 0 )
		{
			pthread_create (&cleanup, NULL, &Clean, NULL);
			Start ();
		} // else if()
		free (line);
	} // while()

	fclose (file);

	for ( int i = 0; i < 2 * NUMPROXIES; i ++ )
	{
		pthread_join (threads[i], NULL);
	}
	pthread_join (cleanup, NULL);
	exit (0);
} // main()
