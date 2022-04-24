#include <QCoreApplication>
#include <QVector>
#include <iostream>
#include <math.h>
#include <chrono>
#include <QWaitCondition>
#include <QMutex>
#include <QReadWriteLock>
#include <condition_variable>
#include <thread>
#include <unistd.h>

#include "statistics.h"

const unsigned TotalData = 60*60*24*365;        // sec * min * hour * days
const unsigned TotalBufferSize = 60*60*24*30;
const unsigned SampleSize = 60*60*24;
QWaitCondition bufferNotEmpty;
QWaitCondition bufferNotFull;
QMutex mutex;
QVector<float> Buffer;
unsigned Comparator = 0;


double getMean(QVector<float>* data, unsigned start_pos) {
    double sum = 0.0;
    for (int i = start_pos; i < start_pos + SampleSize; i++)
        sum += (*data)[i % TotalBufferSize];
    return sum/SampleSize;
}

double median(QVector<float>* data, unsigned start_pos) {
   std::sort((*data).begin() + start_pos, (*data).begin() + start_pos + SampleSize - 1);
   if ((start_pos + SampleSize - 1) % 2 == 0)
      return ((*data)[((start_pos + SampleSize - 1) / 2) - 1] + (*data)[(start_pos + SampleSize - 1) / 2]) / 2.0;
   return (*data)[(start_pos + SampleSize - 1) / 2];
}

void Producer() {
    for (unsigned int i = 0; i < TotalData; i++) {
        mutex.lock();
        if (Comparator == TotalBufferSize) {
            bufferNotFull.wait(&mutex);
        }
        mutex.unlock();

        Buffer[i % TotalBufferSize] = random() % 50 + 50;
        Comparator++;

        mutex.lock();
        if (Comparator >= SampleSize*2)
            bufferNotEmpty.wakeAll();
        mutex.unlock();
    }
}



void Consumer(unsigned int start_pos) {
    unsigned int counter = start_pos;
    while (counter < TotalData) {
        if (counter % SampleSize == 0) {
            mutex.lock();
            if (Comparator < SampleSize)
               bufferNotEmpty.wait(&mutex);
            mutex.unlock();

            std::cout << counter << " of " << TotalData << " ";
            std::cout << "average temperature " << getMean(&Buffer, counter % TotalBufferSize) << " ";
            std::cout << " with median " << median(&Buffer, counter % TotalBufferSize) << " ";
            std::cout << "Current producer index: " << Comparator << "\n";
            std::cout.flush();

            mutex.lock();
            Comparator = Comparator - SampleSize;
            bufferNotFull.wakeAll();
            mutex.unlock();
        }
        counter += SampleSize*2;
    }
}

QReadWriteLock lock;

void Writer() {
    for (unsigned int i = 0; i < TotalData; i++) {
        lock.lockForWrite();
        if (Comparator == TotalBufferSize) {
            bufferNotFull.wait(&lock);
        }
        lock.unlock();

        Buffer[i % TotalBufferSize] = random() % 50 + 50;
        Comparator++;

        lock.lockForWrite();
        if (Comparator >= SampleSize*2)
            bufferNotEmpty.wakeAll();
        lock.unlock();
    }
}

void Reader(unsigned int start_pos) {
    unsigned int counter = start_pos;
    while (counter < TotalData) {
        if (counter % SampleSize == 0) {
            lock.lockForRead();
            if (Comparator < SampleSize)
               bufferNotEmpty.wait(&lock);
            lock.unlock();

            std::cout << counter << " of " << TotalData << " ";
            std::cout << "average temperature " << getMean(&Buffer, counter % TotalBufferSize) << " ";
            std::cout << " with median " << median(&Buffer, counter % TotalBufferSize) << " ";
            std::cout << "Current writer index: " << Comparator << "\n";
            std::cout.flush();

            lock.lockForRead();
            Comparator = Comparator - SampleSize;
            bufferNotFull.wakeAll();
            lock.unlock();
        }
        counter += SampleSize*2;
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    Buffer.resize(TotalBufferSize);

    auto start1 = std::chrono::high_resolution_clock::now();
    for(unsigned int i = 0; i < TotalData; i++) {
        Buffer[i % TotalBufferSize] = random() % 50 + 50;
        if(i % (24*3600) == 0) {
            std::cout << i << " of " << TotalData << " ";
            std::cout << "average temperature " << getMean(&Buffer, i % TotalBufferSize) << " ";
            std::cout << " with median " << median(&Buffer, i % TotalBufferSize) << "\n";
        }
    }
    auto stop1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(stop1 - start1);
    std::cout << duration1.count() << "\n";
    std::cout << "Done in serial mode\n";

    auto start2 = std::chrono::high_resolution_clock::now();
    std::thread p(Producer),c1(Consumer, 0), c2(Consumer, SampleSize);
    p.join();
    c1.join();
    c2.join();
    auto stop2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(stop2 - start2);
    std::cout << duration2.count() << "\n";
    std::cout << "Done in producer/consumer mode\n";

    auto start3 = std::chrono::high_resolution_clock::now();
    std::thread w(Writer), r1(Reader, 0), r2(Reader, SampleSize);
    w.join();
    r1.join();
    r2.join();
    auto stop3 = std::chrono::high_resolution_clock::now();
    auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(stop3 - start3);
    std::cout << duration3.count() << "\n";
    std::cout << "Done in readers writer mode\n";

    std::cout << "|  Serial mode\t|  producer consumer mode\t|  readers writer mode\t\n|  " << duration1.count() << "\t|  " << duration2.count() << "\t\t\t|  " << duration3.count() << "\t\n";
    float producer_consumer_time = duration2.count();
    float readers_writer_time = duration3.count();
    std::cout << "The readers writer mode is " << producer_consumer_time/readers_writer_time << " times faster\n";

    return 0;
}
