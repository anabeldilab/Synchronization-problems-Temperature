# SOA Synchronization problems: Temperature
Made by Anabel Díaz Labrador

# Description
In this project I have programmed a CPU temperature statistics 
calculator for the whole year for each **day**.

To carry out this practice I have worked in QTCreator.


## QTCreator
I have worked on 6.0.2v although it can possibly run on 5v.

I have used QTVector, QMutex, QWaitCondition, QReadWriteLock to make the project.
QTCreator provides a lot of very useful thread management tools.

# How to use?
To run it, simply by having QTCreator, we add the project and click on "Run".

# Code explanation
I have used a given template in the virtual campus to do the practice with a little modification.
I have three branches to divide the code:
* main: It is where the serial mode is.
* producer_consumer: It is where the producer-consumer mode is.
* readers_writer: It is where the readers_writer mode is.

The objective of the project is to check which of these synchronization problems is the most effective at runtime.

I have a 30 days buffer capacity. It's a cyclic buffer.

TotalData is a variable with total data size that I have to pick up. It is a year in seconds `(60*60*24*365)`

I have to calculate the mean and the median of every day on a year.



## Serial mode
The serial mode is a for loop that starts at 0 and ends at *TotalData* in which "every second" I put a random value, between 50 and 100, into the Buffer.

You need to be sure that you are not outside the buffer size. That is so important because *TotalData* size is much bigger than buffer size.

We have an *if* that searches if the iterator is in another day to calculate the mean and the median of a previous day.


## Producer-Consumer mode
The Producer-Consumer mode has three threads. One of these is the producer and the rest are consumers.

In this problem, the producer is too fast, therefore the producer is waiting most of the time for consumers because the buffer is full.

* Producer: It has `mutex.lock()` in two occasions: To check if the buffer is full or to check if the amount he has produced is more than 2 days already. I check if it produce 2 days **to wake** both threads at the same time. Producer also increments a global variable named `Comparator`, this variable is used to know if the buffer is full or "empty" (has less than one day data). 

* Consumer: It has `mutex.lock()` in two occasions: To check if the buffer is "empty" or to decrement the Comparator one day in seconds and wake the producer. The consumer increases day by day because the producer is too fast, therefore we can afford that.

## Readers-Writer mode
The Readers-Writer mode has three threads. One of these is the writer and the rest are readers.

As the Producer-Consumer mode, the writer is also too fast for the readers.

Writer and readers work the same way as producer-consumer problem but readers work with "shared lock" LockForRead, and the writer works with "unique lock" LockForWrite.


# Comparative tables

I have done the tests on linuxOS.

|        | Serial mode   | Producer-Consumer mode   | Readers-Writer mode   |
| :----: | :-----------: | :----------------------: | :-------------------: |
| µs     | 14363335      | 93213797                 | 7371308               |
| s      | 14.36         | 93.213                   | 7.37                  | 

I think Readers-Writer mode is faster than others in that problem because Readers can work at the same time without locking themselves because they aren't reading the same data at all. However in Producer-Consumer mode each thread is locking the read of the threads every time.