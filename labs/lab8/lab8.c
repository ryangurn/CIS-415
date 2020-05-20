#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 255
#define MAX_QUEUES 10

typedef struct {
  int ticketNum;
  char *dish;
} mealTicket;

typedef struct {
  char name[MAX_NAME];
  mealTicket * const buffer;
  int head;
  int tail;
  const int length;
} MTQ;

static MTQ *registry[MAX_QUEUES];

MTQ initMTQ(char *name, int length)
{
  //setup mealtickets
  mealTicket *buf = malloc(length * sizeof(mealTicket));
  mealTicket nul = {.ticketNum = 0, .dish = NULL};

  // set all slots to the nul ticket
  for (size_t i = 0; i < length; i++) {
    buf[i] = nul;
  }
  // set the last ticket to ticketNum of -1
  buf[length-1].ticketNum = -1;

  // init the queue
  MTQ queue = {.head = 0, .tail = 0, .buffer = buf, .length = length };
  //copy name
  strcpy(queue.name, name);
  return queue; // return the queue
}

int enqueue(char *MTQ_ID, mealTicket *MT)
{
  MTQ *queue;
  int last = MAX_QUEUES-1;

  for (size_t i = 0; i < MAX_QUEUES; i++) {
    queue = registry[i]; // assign the specific queue

    // are we looking at the correct queue
    if(strcmp(queue->name, MTQ_ID) == 0)
    {
      break; // exit the loop if so
    }

    if(i == last)
    {
      return 0; // we have checked everything
    }

  } // end for

  int tail = queue->tail;
  if(queue->buffer[tail].ticketNum == -1)
  {
    return 0;
  }
  else
  {
    MT->ticketNum = tail+1;
    queue->buffer[tail] = *MT;
    tail++;

    if(tail == queue->length)
    {
      queue->tail = 0;
    }
    else
    {
      queue->tail = tail;
    }

  }
  return 1;
}

int dequeue(char *MTQ_ID, int ticketNum, mealTicket *MT)
{
  MTQ *queue;
  int last = MAX_QUEUES-1;

  for (size_t i = 0; i < MAX_QUEUES; i++) {
    queue = registry[i];

    if(strcmp(queue->name, MTQ_ID) == 0)
    {
      break;
    }

    if(i == last)
    {
      return 0;
    }

  } // end for

  int head = queue->head;
  int tail = queue->tail;
  int ticket = queue->buffer[head].ticketNum;

  if(ticket <= 0)
  {
    *MT = queue->buffer[head];
    return 0;
  }
  else if (ticket == ticketNum)
  {
    *MT = queue->buffer[head];
    mealTicket nul = { .ticketNum = 0, .dish = NULL };
    queue->buffer[tail] = nul;
    nul.ticketNum = -1;
    queue->buffer[head] = nul;

    if(head == queue->length - 1)
    {
      queue->head = 0;
    }
    else
    {
      queue->head = head+1;
    }
  }
  else
  {
    return 0;
  }

  return 1;
}

int main()
{
  MTQ breakfast = initMTQ("Breakfast", 4);
  registry[0] = &breakfast;

  mealTicket burrito = { .ticketNum = -2, .dish = "Breakfast Burrito"};
  mealTicket poachedEgg = { .ticketNum = -2, .dish = "Poached Eggs"};
  mealTicket bagel = { .ticketNum = -2, .dish = "Bagel"};
  enqueue("Breakfast", &burrito);
  enqueue("Breakfast", &poachedEgg);
  enqueue("Breakfast", &bagel);


  MTQ lunch = initMTQ("Lunch", 4);
  registry[1] = &lunch;

  mealTicket sandwich = { .ticketNum = -2, .dish = "Sandwich"};
  mealTicket burger = { .ticketNum = -2, .dish = "Burger"};
  mealTicket pizza = { .ticketNum = -2, .dish = "Pizza"};
  enqueue("Lunch", &sandwich);
  enqueue("Lunch", &burger);
  enqueue("Lunch", &pizza);

  MTQ dinner = initMTQ("Dinner", 4);
  registry[2] = &dinner;

  mealTicket pasta = { .ticketNum = -2, .dish = "Pasta"};
  mealTicket potstickers = { .ticketNum = -2, .dish = "Pot Stickers"};
  mealTicket ramen = { .ticketNum = -2, .dish = "Ramen"};
  enqueue("Dinner", &pasta);
  enqueue("Dinner", &potstickers);
  enqueue("Dinner", &ramen);

  MTQ bar = initMTQ("Bar", 4);
  registry[3] = &bar;

  mealTicket rum = { .ticketNum = -2, .dish = "Rum"};
  mealTicket wine = { .ticketNum = -2, .dish = "Wine"};
  mealTicket wiskey = { .ticketNum = -2, .dish = "Wiskey"};
  enqueue("Bar", &rum);
  enqueue("Bar", &wine);
  enqueue("Bar", &wiskey);


  // completeing test cases
  // Dequeue when a queue is empty
  int i = 1;
  while (i < 5) {

    for (size_t j = 0; j < 4; j++) {
      mealTicket mt;
      char *name = registry[j]->name;
      dequeue(name, i, &mt);
      printf("Queue: %s - Ticket Number: %d - Dish: %s\n", name, mt.ticketNum, mt.dish);
    }
    i++;
  }

  mealTicket mt;
  if(dequeue(registry[0]->name, registry[0]->head+1, &mt) == 0)
  {
    printf("Test Case: A - Result: Success\n");
  }
  else
  {
    printf("Test Case: A - Result: Fail\n");
  }

  //Dequeue when a queue is full
  enqueue("Breakfast", &burrito);
  enqueue("Breakfast", &poachedEgg);
  enqueue("Breakfast", &bagel);
  if(dequeue(registry[0]->name, registry[0]->head+1, &mt) == 1)
  {
    printf("Test Case: B - Result: Success\n");
  }
  else
  {
    printf("Test Case: B - Result: Fail\n");
  }

  // Enqueue when a queue is full
  enqueue("Breakfast", &burrito);
  if(enqueue("Breakfast", &burrito) == 0)
  {
    printf("Test Case: C - Result: Success\n");
  }
  else
  {
    printf("Test Case: C - Result: Fail\n");
  }

  // Enqueue when a queue if empty
  if(enqueue("Bar", &rum) == 1)
  {
    printf("Test Case: D - Result: Success\n");
  }
  else
  {
    printf("Test Case: D - Result: Fail\n");
  }

  // free all the things
  free(registry[0]->buffer);
  free(registry[1]->buffer);
  free(registry[2]->buffer);
  free(registry[3]->buffer);
}
