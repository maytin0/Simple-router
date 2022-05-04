#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SPACE 1048576 
//SPACE is 1MB

void enqueue(struct packet* ptrPack, struct queue* queue);
void drop(int place, struct packet* ptrPack, struct queue* queue);
void dequeue(struct packet* ptrPack, struct queue* queue);

struct packet {

	int dst_ip; // Destination ip address.
	int len; // Packet length
	int priority; //0: low priority, 1: high priority

};

struct queue {
	int queue[10488][3];// queue 
	int sumLen;  //total length of queue
	int tail;    //tail position number	
	
	int lowSize; //low priority packets' size
};

struct queue queue1 = { {0},0,0,0 };
struct queue queue2 = { {0},0,0,0 };
struct queue queue3 = { {0},0,0,0 };
struct queue queue4 = { {0},0,0,0 };

struct data {

	unsigned int numDrop;//number of dropped packets
	unsigned int high;   // high priority packets
	unsigned int low;	//low priority packets 
	long double totalBytes; // total bytes sended // Long double because when execution time is big then it gets really big
};

struct data data = { 0,0,0,0 };

void enqueue(struct packet* ptrPack, struct queue* queue)
{
	//printf("tail:%3d,  sumLen:%7d , owSize:%6d , SPACE:%7d\n", queue->tail, queue->sumLen, queue->lowSize, SPACE - queue->sumLen); //Check
		
		if ((ptrPack->len) <= (SPACE - queue->sumLen)) {
			//If there is enough space then directly this part will be performed
			
			//We are assigning packet values to queue
			queue->queue[queue->tail][0] = ptrPack->dst_ip; 
			queue->queue[queue->tail][1] = ptrPack->len;
			queue->queue[queue->tail][2] = ptrPack->priority;
			queue->tail++; //Tail is increased
			queue->sumLen += ptrPack->len; // Total size is increased

			if (ptrPack->priority == 0) {
				 // I have added some extra data to ease rest pa 
				queue->lowSize += ptrPack->len;
				data.low++;

			}
			else {
				data.high++;
			}
			
			
		}
		else if ((ptrPack->priority==1) && ((ptrPack->len) <= (queue->lowSize)) && ((ptrPack->len) <= (SPACE - (queue->sumLen)))) {
			int sum_low = 0;
			int count_q;

			count_q = (queue->tail);

			do {

				if (queue->queue[count_q][2] == 1) {
					count_q--;
					continue;
				}
				else {
					sum_low += ptrPack->len;
					drop(count_q, ptrPack, queue);
					count_q--; //drop(queue->queue[queue->][count_q])
				}
			} while ((sum_low <= ptrPack->len) && (count_q >= 0));
			queue->sumLen += ptrPack->len;
			queue->queue[queue->tail][0] = ptrPack->dst_ip;
			queue->queue[queue->tail][1] = ptrPack->len;
			queue->queue[queue->tail][2] = ptrPack->priority;
			(queue->tail);

			
			data.high++;

		}
		else {
			// This part is not necessary but i choosed to use this way
			queue->queue[queue->tail][0] = ptrPack->dst_ip;
			queue->queue[queue->tail][1] = ptrPack->len;
			queue->queue[queue->tail][2] = ptrPack->priority;
			// Adding it to tail and not incrementing tail value
			drop(queue->tail, ptrPack, queue);
		}

}

void dequeue(struct packet* ptrPack, struct queue* queue)
{
	// Drop from head
	drop(0, ptrPack, queue);
}

void drop(int place, struct packet* ptrPack, struct queue* queue)
{
	int i;
	int hold[3] = { 0 };
	int* ptrHold = hold; // For holding the packet and to assign it one upper block
	
	if (queue->tail == place) {
		// Not necessary but i used it 
		// It removes nothing just deletes the packet which we cannot add to queue

		queue->queue[queue->tail][0] = 0;
		queue->queue[queue->tail][1] = 0;
		queue->queue[queue->tail][2] = 0;

	}
	else {
		data.totalBytes += ptrPack->len;
		queue->sumLen -= ptrPack->len;
		queue->queue[place][0] = 0;
		queue->queue[place][1] = 0;
		queue->queue[place][2] = 0;



		if (ptrPack->priority == 0) queue->lowSize -= ptrPack->len;
		
		for (i = (place + 1); i < queue->tail; i++) {
			//Because we removed one packet from queue, queue has a gap we have to shift packets up -in verticular form- 

			hold[0] = queue->queue[i][0]; // Holding part
			hold[1] = queue->queue[i][1];
			hold[2] = queue->queue[i][2];
			queue->queue[i][0] = 0;
			queue->queue[i][1] = 0;
			queue->queue[i][2] = 0;
			queue->queue[i - 1][0] = hold[0]; // Assigning part
			queue->queue[i - 1][1] = hold[1];
			queue->queue[i - 1][2] = hold[2];

		}
		queue->tail--;  // One packet dropped so tail will be one less
		data.numDrop++; // Total dropped packets
	}
}

void random_packet_generator(struct packet* ptrPack)
{
	// Random packet generator
	int x;
	int randPort;
	x = (1 + (rand() % 254));
	//printf("x: %d \n", x); // Check
	randPort = (1 + (rand() % 4));
	switch (randPort)
	{
	case 1: ptrPack->dst_ip = (0x0A001400 + x); break;
	case 2: ptrPack->dst_ip = (0x0A000C00 + x); break;
	case 3: ptrPack->dst_ip = (0x0A003200 + x); break;
	case 4: ptrPack->dst_ip = (0x0A004600 + x); break;
	}

	//printf("%X", pack.dst_ip); // Check
	ptrPack->len = (100 + (rand() % 1401));
	ptrPack->priority = (rand() % 2);
	//Packet has been created
}

int main()
{

	float cr;
	int eTime;


	struct packet pack;
	struct packet* ptrPack;
	ptrPack = &pack;

	int count, i;
	clock_t t1, t2, t3, t4;
	int diftime, diftime2 = 0;

	float enqueue_rate, dequeue_rate;
	int enq_count, deq_count;

	//cr will be between [0,100]
	printf("Congestion Rate: ");
	scanf_s("%f", &cr);

	// CR = (EnqueueRate � DequeueRate) / EnqueueRate 
	// CR Between 0 and 1; 100CR = cr
	// CR*EnqueueRate = EnqueueRate � DequeueRate
	// EnqueueRate(1-CR) = DequeueRate
	// Set Enqueue to 100 to handle rates easily
	// 100-(100*CR) = DequeueRate
	// 100 - cr = DequeueRate
	
	if (cr == 0) {
		enqueue_rate = 50;
		dequeue_rate = 50;
	}
	else if (cr == 100) {
		dequeue_rate = 0;
		enqueue_rate = 100;
	}
	else {
		enqueue_rate = 100;
		dequeue_rate = 100 - (int)(cr);
	}

	printf("Total simulation time(seconds): ");
	scanf_s("%d", &eTime);

	// port1 : 10.0.20.x -> 0x0A0014rr	 	x(rr) between [1,254]
	// port2 : 10.0.12.x -> 0x0A000Crr        
	// port3 : 10.0.50.x -> 0x0A0032rr     
	// port4 : 10.0.70.x -> 0x0A0046rr

	srand(time(NULL));
	t3 = clock() - 1000; // Substracting 1 second
	t1 = clock(); // Time for compare
	do {


		t4 = clock();
		if (((t4 - t3) / CLOCKS_PER_SEC) == 1) {
			//Data that will be printed
			printf("Queue1 Size: %d\nQueue2 Size: %d\nQueue3 Size: %d\nQueue4 Size: %d\n", queue1.sumLen, queue2.sumLen, queue3.sumLen, queue4.sumLen);
			printf("Number of packets dropped: %d\n", data.numDrop);
			printf("Number of high / low priority packets routed successfully: %d / %d\n", data.high, data.low);
			printf("Number of bytes routed succesfully: %.0Lf\n\n", data.totalBytes);
			t3 = t3 + 1000;// I am adding 1000 since my system has 1000 clocks ticks per second which means i am adding 1 second
			
			/*
			-queue sizes,
			-number of packets dropped
			-number of high / low priority packets routed successfully
			-number of bytes routed successfully
			*/
		

		}


		for (i = 0; i < (enqueue_rate + dequeue_rate); i++) // we are looping for total rate 
		{
			if (i < enqueue_rate) {  // for enqueue
				
				random_packet_generator(ptrPack);
				
				// for related port we will call function enqueue
				if ((ptrPack->dst_ip & 0x00001400) == 0x00001400) {
					//printf("port 1\n\n"); //Check
					enqueue(ptrPack, &queue1);
				}
				else if ((ptrPack->dst_ip & 0x00000C00) == 0x00000C00) {
					//printf("port 2\n\n");  //Check
					enqueue(ptrPack, &queue2);
				}
				else if ((ptrPack->dst_ip & 0x00003200) == 0x00003200) {
					//printf("port 3\n\n"); //Check
					enqueue(ptrPack, &queue3);
				}
				else {
					//printf("port 4\n\n"); //Check
					enqueue(ptrPack, &queue4);
				}
			}
			else
			{	// For dequeue
				// I choosed to drop one by one from the ports 
				switch ((i % 4)) {
				case 0: {	// port1	
					ptrPack->dst_ip = queue1.queue[0][0];
					ptrPack->len = queue1.queue[0][1];
					ptrPack->priority = queue1.queue[0][2];
					dequeue(ptrPack, &queue1); 
					break; }
				case 1: {  // port2
					ptrPack->dst_ip = queue2.queue[0][0];
					ptrPack->len = queue2.queue[0][1];
					ptrPack->priority = queue2.queue[0][2];
					dequeue(ptrPack, &queue2); 
					break;}
				case 2: {	// port3
					ptrPack->dst_ip = queue3.queue[0][0];
					ptrPack->len = queue3.queue[0][1];
					ptrPack->priority = queue3.queue[0][2];
					dequeue(ptrPack, &queue3); 
					break;}
				case 3: {	// port4
					ptrPack->dst_ip = queue4.queue[0][0];
					ptrPack->len = queue4.queue[0][1];
					ptrPack->priority = queue4.queue[0][2];
					dequeue(ptrPack, &queue4); 
					break;}
				}
			}
		}

		t2 = clock();
		diftime = (t2 - t1) / CLOCKS_PER_SEC; //To check time difference -but be careful its in seconds


	} while (diftime <= (eTime)); // if we exceed given time loop wont work
	printf("----------------------------------------");
	return 0;
}