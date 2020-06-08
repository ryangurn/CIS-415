#define  _GNU_SOURCE
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#define true 1
#define false 0

#define MAXBUFFERS 10
#define MAXENTRIES 100
#define MAXPUBLISHERS 50
#define MAXSUBSCRIBERS 50
#define MAXTOPICS 1000

char *left_trim(char *s){ while(isspace(*s)) s++; return s; }
char *right_trim(char *s){ char *back = s + strlen(s); while(isspace(*--back)); *(back+1) = '\0'; return s; }
char *trim(char *s){ return right_trim(left_trim(s));  }


typedef struct {
	int entryNum;              // entry number
	struct timeval timeStamp;  // created time
	int pubID;                 // publisher id
	char photoURL[1000];       // url of topic
	char photoCaption[1000];   // photo caption
} topicEntry;

typedef struct {
	int id;										// queues id
	char name[50];            // queues name
	int entryNum;             // int to reference the entryNum
	int head;                 // head of the queue
	int tail;                 // tail of the queue
	int max;                  // length of the queue
	topicEntry *t;            // topic array
	pthread_mutex_t mutex;    // mutex lock
} topicEntryQueue;

typedef struct {
	pthread_t thread;         // thread
	int flag;                 // free or nah
	char location[1024];
	// we might need some char or something here
} threadArguements;

topicEntryQueue Queues[MAXENTRIES];
topicEntry arr[MAXTOPICS][MAXENTRIES+1];

pthread_mutex_t publisher_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t subscriber_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition;
pthread_t publishers[MAXPUBLISHERS];
pthread_t subscribers[MAXSUBSCRIBERS];
pthread_attr_t attributes;

pthread_mutex_t mutex[MAXBUFFERS];
sem_t full[MAXBUFFERS];
sem_t empty[MAXBUFFERS];

threadArguements publisherArgs[MAXPUBLISHERS];
threadArguements subscriberArgs[MAXSUBSCRIBERS];

int diffDelta;
int entryValue = 1;
static int status = 0;
static int topics = 0;
topicEntry null = {-1};

int enqueue(int topicEntryQueueID, topicEntry TE)
{
	int i;																						// iterator
	int found = -1;																		// have we found the queue
	topicEntryQueue tmp;															// tmp location to store queue
	for (i = 0; i < MAXTOPICS; i++)
	{
		if(Queues[i].id == topicEntryQueueID)
		{
			tmp = Queues[i];
			found = i;
			break;
		}
	} // for()

	// if we have not found the item
	if (found == -1)
	{
		return 0;
	}

	if (tmp.t[tmp.tail].entryNum == -1)
	{
		return 0;
	}
	else
	{
		// insert the topic into the queue and timestamp it
		TE.entryNum = tmp.entryNum;
		gettimeofday(&TE.timeStamp, NULL);
		tmp.t[tmp.tail] = TE;

		if (tmp.tail + 1 > tmp.max)
		{
			tmp.tail = 0;
		}
		else
		{
			tmp.tail++;
		}

		tmp.entryNum++;
	}

	return 1;

}

int ts_enqueue(int topicEntryQueueID, topicEntry TE)
{
	int i;
	int failed;
	int found = -1;

	time_t entry;

	for ( i = 0; i < MAXTOPICS; i++)
	{
		if (Queues[i].id = topicEntryQueueID)
		{
			found = i;
			break;
		}
	} // for()

	// not found
	if (found == -1)
	{
		return 0;
	}

	pthread_mutex_lock(&Queues[i].mutex); // locked

	failed = enqueue(topicEntryQueueID, TE); // enqueue it

	pthread_mutex_unlock(&Queues[i].mutex); // unlocked

	if (failed == 0)
	{
		return 0; // failed
	}
	else
	{
		return 1; // success
	}
}

int get_entry(int topicEntryQueueID, int entry, topicEntry *TE)
{
	int i;
	int found = -1;

	topicEntryQueue tmp;
	for (i = 0; i < MAXTOPICS; i++) {
		if (Queues[i].id == topicEntryQueueID)
		{
			tmp = Queues[i];
			found = i;
			break;
		}
	} // for()

	if (found == -1)
	{
		return 0;
	}

	if (tmp.tail == tmp.head)
	{
		return 0;
	}

	int k = 0;
	// find the next topic and save it to the given ptr
	for (k; k < tmp.max; k++)
	{
		if (tmp.t[k].entryNum > entry)
		{
			if (tmp.t[k].entryNum > entry)
			{
				strcpy(TE->photoCaption, tmp.t[k].photoCaption);
				strcpy(TE->photoURL, tmp.t[k].photoURL);
				TE->pubID = tmp.t[k].pubID;
				TE->entryNum = tmp.t[k].entryNum;
				TE->timeStamp = tmp.t[k].timeStamp;
			}

			if (k == tmp.max)
			{
				strcpy(TE->photoCaption, tmp.t[tmp.head].photoCaption);
				strcpy(TE->photoURL, tmp.t[tmp.head].photoURL);
				TE->pubID = tmp.t[tmp.head].pubID;
				TE->entryNum = tmp.t[tmp.head].entryNum;
				TE->timeStamp = tmp.t[tmp.head].timeStamp;
			}
			else
			{
				strcpy(TE->photoCaption, tmp.t[k+1].photoCaption);
				strcpy(TE->photoURL, tmp.t[k+1].photoURL);
				TE->pubID = tmp.t[k+1].pubID;
				TE->entryNum = tmp.t[k+1].entryNum;
				TE->timeStamp = tmp.t[k+1].timeStamp;
			}

		} // if()
	} // for()
	return 1;
}

int ts_get_entry(int topicEntryQueueID)
{
	int i;
	int found = -1;

	for (i = 0; i < MAXTOPICS; i++)
	{
		if (Queues[i].id == topicEntryQueueID)
		{
			found = i;
			break;
		}
	} // for()

	if (found == -1)
	{
		return 0;
	}

	pthread_mutex_lock(&Queues[found].mutex);

	topicEntry *topic;
	int current = Queues[found].entryNum;
	int k = 0;
	for (k; k <= current; k++)
	{
		get_entry(topicEntryQueueID, k, topic);
	}

	pthread_mutex_unlock(&Queues[found].mutex);

	return 1;
}

int dequeue()
{
	int i = 0;
	for (i; i < MAXTOPICS; i++)
	{
		int k;
		if (Queues[i].name != NULL)
		{
			k = Queues[i].head;
			for (k; k < Queues[i].tail; k++)
			{
				if (strcmp(Queues[i].t[k].photoCaption, "\0") != 0)
				{
					struct timeval curr_time;
					gettimeofday(&curr_time, NULL);

					double diff = (curr_time.tv_sec - Queues[i].t[k].timeStamp.tv_sec) +
					((curr_time.tv_usec - Queues[i].t[k].timeStamp.tv_usec)/1000000.0);

					if (diff >= diffDelta)
					{
						strcpy(Queues[i].t[k].photoCaption, "\0");
						strcpy(Queues[i].t[k].photoURL, "\0");
						Queues[i].t[k].entryNum = 0;
						Queues[i].t[k].pubID = 0;

						if (Queues[i].head == Queues[i].max-1)
						{
							Queues[i].head = 0;
						}
						else
						{
							Queues[i].head++;
						}

					} // if()

				} // if()
			} // for()
		}
		else
		{
			continue;
		}
	}
}

void *Publisher(void *args)
{
	char *line = NULL;
	ssize_t number;
	size_t length = 0;

	FILE *configuration;

	pthread_mutex_lock(&publisher_lock);

	while(((threadArguements *) args)->location == "\0")
	{
		pthread_cond_wait(&condition, &publisher_lock);
	}

	configuration = fopen(((threadArguements *) args)->location, "r");
	number = getline(&line, &length, configuration);

	do
	{

		int i, j = 0;
		char *token;
		char *save_ptr;
		char str[100] = "";
		char *strArr[1000];

		token = strtok_r(line, " ", &save_ptr);
		while (token != NULL)
		{
			if (token[0] == '"')
			{
				int i;
				int iter = 0;
				for (i = 1; token[i] != '"'; i++)
				{
					str[iter] = token[i];
					iter++;
				}
				strArr[j] = str;
			}
			else
			{
				strArr[j] = token;
			}


			j++;

			token = strtok_r(NULL, " ", &save_ptr);
		} // while()

		if (strcmp(trim(strArr[0]), "put") == 0)
		{
			int id;
			// typedef struct {
			// 	int entryNum;              // entry number
			// 	struct timeval timeStamp;  // created time
			// 	int pubID;                 // publisher id
			// 	char photoURL[1000];       // url of topic
			// 	char photoCaption[1000];   // photo caption
			// } topicEntry;
			id = atoi(strArr[1]);
			topicEntry TE = {
				.entryNum = entryValue,
				.pubID = id
			};
			entryValue++;
			strcpy(TE.photoURL, strArr[2]);
			strcpy(TE.photoCaption, strArr[3]);

			ts_enqueue(id, TE);

			printf("Proxy Thread: %ld - type: Publisher- Executed Command: put\n", pthread_self());
		}
		else if (strcmp(trim(strArr[0]), "sleep") == 0)
		{
			int sleep = atoi(strArr[1]);
			usleep(sleep);
			printf("Proxy Thread: %ld - type: Publisher- Executed Command: sleep\n", pthread_self());
		}
		else if (strcmp(trim(strArr[0]), "stop") == 0)
		{
			printf("Proxy Thread: %ld - type: Publisher- Executed Command: stop\n", pthread_self());
			break;
		}
		else
		{
			printf("Proxy Thread | type(publisher): unrecognized cmd: %s\n", trim(strArr[0]));
		}

	} while((number = getline(&line, &length, configuration)) != -1);

	printf("Proxy Thread: %ld - type: Publisher\n", pthread_self());
	strcpy(((threadArguements *) args)->location, "\0");
	fclose(configuration);

	pthread_mutex_unlock(&publisher_lock);
}

void *Subscriber(void *args)
{
	char *line = NULL;
	ssize_t number;
	size_t length = 0;

	FILE *configuration;

	pthread_mutex_lock(&subscriber_lock);
	while (((threadArguements *) args)->location == "\0")
	{
		pthread_cond_wait(&condition, &subscriber_lock);
	}

	configuration = fopen(((threadArguements *) args)->location, "r");
	number = getline(&line, &length, configuration);

	do
	{
		int i = 0;
		int j = 0;
		char *token;
		char *save_ptr;
		char str[100];
		char *strArr[1000];

		token = strtok_r(line, " ", &save_ptr);
		while ( token != NULL )
		{
			if (token[0] == '"')
			{
				int i;
				int iter = 0;
				for (i = 1; token[i] != '"'; i++)
				{
					str[iter] = token[i];
					iter++;
				}
				strArr[j] = str;
			}
			else
			{
				strArr[j] = token;
			}

			j++;
			token = strtok_r(NULL, " ", &save_ptr);

		} // while()


		if (strcmp(trim(strArr[0]), "get") == 0)
		{
			ts_get_entry(atoi(strArr[1]));
			printf("Proxy Thread: %ld - type: Subscriber - Executed command : get\n", pthread_self());
		}
		else if (strcmp(trim(strArr[0]), "sleep") == 0)
		{
			usleep(atoi(strArr[1]));
			printf("Proxy thread: %ld - Type: Subscriber - Executed Command: sleep\n", pthread_self());
		}
		else if (strcmp(trim(strArr[0]), "stop") == 0)
		{
			printf("Proxy Thread : %ld - Type: Subscriber - executed command : stop\n", pthread_self());
			break;
		}
		else
		{
			printf("Proxy Thread | type(subscriber): unrecognized cmd: %s\n", trim(strArr[0]));
		}

	} while((number = getline(&line, &length, configuration)) != -1);

	printf("Proxy thread: %ld | type : Subscriber\n", pthread_self());
	strcpy(((threadArguements *) args)->location, "\0");
	fclose(configuration);

	pthread_mutex_unlock(&subscriber_lock);
}

void *Cleaner(void *args)
{
	struct timeval start, curr;
	gettimeofday(&start, NULL);
	while (true)
	{
		//get the time of gettimeofday
		gettimeofday(&curr, NULL);

		// get elapsed
		double elapsed = (curr.tv_sec - start.tv_sec) + ((curr.tv_usec - start.tv_usec)/1000000.0);

		if (elapsed >= diffDelta + 5)
		{
			dequeue();
			gettimeofday(&start, NULL);
		}
		else
		{
			sched_yield();
		}
	}
}

int check_arguments(char *argv[])
{
	if (argv[1] == NULL || argv[2] != NULL)
	{
		return true; //err
	}
	return false;
}

int main(int argc, char *argv[])
{
	if (check_arguments(argv))
	{
		printf("Error: invalid args\n");
		exit(1);
	}

	FILE *file = fopen(argv[1], "r");
	if (file == NULL)
	{
		printf("Error: failed to open file.\n");
		exit(1);
	}

	size_t length = 0;
	int numberOfPublishers = 0;
	int numberOfSubscribers = 0;
	pthread_attr_init(&attributes);
	pthread_t cleanerThread;

	while (file != NULL && status == 0) {
		char *line = NULL;
		char cmd[32];
		getline(&line, &length, file);
		sscanf(line, "%15s", cmd);

		printf("--> %s\n",line);
		fflush(stdout);
		printf("---> %s\n",cmd);
		fflush(stdout);

		if (strcmp(cmd, "create") == 0)
		{
			int topicID = 0;
			int queueLength;
			char name[256] = "";

			sscanf(line, "create topic %d \"%127[^\"]\" %d", &topicID, name, &queueLength);
			if (topicID <= 0 || queueLength <= 0 || strcmp(name, "") == 0)
			{
				printf("Error: could not create topic, null information\n");
				status = 1;
			}
			else
			{
				topicEntryQueue teq = {
					.id = topicID,
					.head = 0,
					.tail = 0,
					.max = queueLength,
					.mutex = PTHREAD_MUTEX_INITIALIZER,
				};

				// for (size_t i = 0; i < queueLength+1; i++) {
				// 	teq.t[i] = (topicEntry*)malloc(sizeof(topicEntry));
				// }

				teq.t = (topicEntry *)malloc(sizeof(topicEntry) * (queueLength + 1));
				teq.t[queueLength].entryNum = -1;
				Queues[topics] = teq;
				topics++;
			}
		}
		else if (strcmp(cmd, "query") == 0)
		{
			char variable[128];
			int k;
			sscanf(line, "query %63s", variable);

			if (strcmp(variable, "publishers") == 0)
			{
				for (k = 0; k < numberOfPublishers; k++)
				{
					if (publisherArgs[k].flag != 0)
					{
						threadArguements argz = publisherArgs[k];
						printf("Publisher(%d) - File: %s\n", argz.flag, argz.location);
					}
				}
			}
			else if(strcmp(variable, "subscribers") == 0)
			{
				for (k = 0; k < numberOfSubscribers; k++)
				{
					if (subscriberArgs[k].flag != 0)
					{
						threadArguements argz = subscriberArgs[k];
						printf("Subscriber(%d) - File: %s\n", argz.flag, argz.location);
					}
				}
			}
			else if (strcmp(variable, "topics") == 0)
			{
				for (k = 0; k < MAXTOPICS; k++)
				{
					if (Queues[k].max != 0)
					{
						topicEntryQueue tq = Queues[k];
						printf("Queue(%d) - Length: %d\n", tq.id, tq.max);
					}
				}
			}
			else
			{
				printf("Error: command was not recognised for querying: %s\n", variable);
				status = 1;
			}
		}
		else if (strcmp(cmd, "add") == 0)
		{
			char variable[128];
			char filename[128] = "";
			int result = 0;

			sscanf(line, "add %63s \"%127[^\"]\"", variable, filename);
			if (strcmp(variable, "publisher") == 0)
			{
				int found = 0;
				int i = 0;
				for (i; i < MAXPUBLISHERS; i++)
				{
					if(publisherArgs[i].flag == 1)
					{
						found = 1;
						break;
					} // if()
				} // for()

				if (found == 0)
				{
					for (i; i < MAXPUBLISHERS; i++) {
						if (publisherArgs[i].flag == 0)
						{
							found = 1;
							break;
						} // if()
					} // for()
				} // if()


				publisherArgs[i].flag = 2;
				strcpy(publisherArgs[i].location, filename);
				pthread_create(&publisherArgs[i].thread, NULL, Publisher, (void *)&publisherArgs[i]);

				publisherArgs[i].flag = 1;
			}
			else if (strcmp(variable, "subscriber") == 0)
			{
				int found = 0;
				int i = 0;
				for (i; i < MAXSUBSCRIBERS; i++)
				{
					if(subscriberArgs[i].flag == 1)
					{
						found = 1;
						break;
					} // if()
				} // for()

				if (found == 0)
				{
					for (i; i < MAXSUBSCRIBERS; i++) {
						if (subscriberArgs[i].flag == 0)
						{
							found = 1;
							break;
						} // if()
					} // for()
				} // if()

				subscriberArgs[i].flag = 2;
				strcpy(subscriberArgs[i].location, filename);
				pthread_create(&subscriberArgs[i].thread, NULL, Subscriber, (void *)&subscriberArgs[i]);

				subscriberArgs[i].flag = 1;
			}
			else
			{
				printf("Error: unrecognized item to append: %s\n", variable);
				result = 1;
			}
		}
		else if (strcmp(cmd, "delta") == 0)
		{
			float de = 0;
			sscanf(line, "delta %f", &de);
			if (de > 0)
			{
				printf("Delta = %f\n", de);
				diffDelta = de;
			}
			else
			{
				printf("Error: invalid delta\n");
			}
		}
		else if (strcmp(cmd, "start") == 0)
		{
			pthread_create(&cleanerThread, NULL, Cleaner, NULL);
			pthread_join(cleanerThread, NULL);

			pthread_cond_broadcast(&condition);
		}
		else
		{
			char cmd[64];
			sscanf(line, "%s", cmd);
			printf("Error: unrecognized cmd: %s\n", line);
			status = 1;
		}
		free(line);
	}

	if (status == 0)
	{
		for (size_t i = 0; i < MAXPUBLISHERS; i++) {
			if (publisherArgs[i].flag != 0)
			{
				pthread_join(publisherArgs[i].thread, NULL);
			}
		}

		for (size_t i = 0; i < MAXSUBSCRIBERS; i++) {
			if (subscriberArgs[i].flag != 0)
			{
				pthread_join(subscriberArgs[i].thread, NULL);
			}
		}

		for (size_t i = 0; i < MAXTOPICS; i++) {
			free(Queues[i].t);
		}
	}
	fclose(file);
}
