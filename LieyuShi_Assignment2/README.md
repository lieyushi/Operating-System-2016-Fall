-----------------------------------------------------------------
This is assignment 2 for Operating System COSC 6360
Author: Lieyu Shi
Date: Nov 7th, 2016

-----------------------------------------------------------------
Instructions:
	make
	./main test1.txt
	make clean
	./main test2.txt

-----------------------------------------------------------------
Able to read from files and recognize different groups, e.g., it 
can work for test2.txt which there're 'A' and 'B' groups

-----------------------------------------------------------------
Use a semaphore to act as mutual exclusion for different groups,
and besides, with a pthread_mutex_t the computation can be taken
as an atomic.
