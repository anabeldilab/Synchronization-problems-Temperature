#include <QCoreApplication>
#include <QVector>
#include <iostream>
#include <math.h>
#include <chrono>
#include <QWaitCondition>
#include <QMutex>
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
            std::cout << "Current index: " << Comparator << "\n";
            std::cout.flush();

            mutex.lock();
            Comparator = Comparator - SampleSize;
            bufferNotFull.wakeAll();
            mutex.unlock();
        }
        counter += SampleSize*2;
    }
}


int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    auto start = std::chrono::high_resolution_clock::now();
    for(unsigned int i = 0; i < TotalData; i++) {
        Buffer.push_back(random() % 50 + 50);
        if(i % (24*3600) == 0) {
            Statistics s(Buffer);
            std::cout << i << " of " << TotalData << " ";
            std::cout << "average temperature " << s.getMean() << " ";
            std::cout << " with median " << s.median() << "\n";
            Buffer.erase(Buffer.constBegin(),Buffer.constEnd());
        }
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration1.count() << "\n";
    std::cout << "Done in serial mode\n";



    Buffer.resize(TotalBufferSize);
    auto start1 = std::chrono::high_resolution_clock::now();
    std::thread p(Producer),c1(Consumer, 0), c2(Consumer, SampleSize);
    p.join();
    c1.join();
    c2.join();
    auto stop1 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(stop1 - start1);
    std::cout << duration2.count() << "\n";
    std::cout << "Done in producer/consumer mode\n";

    std::cout << "|Serial mode\t|  producer consumer mode\t|\n|" << duration1.count() << "\t|  " << duration2.count() << "\t\t\t|\n";
    float serial_time = duration1.count();
    float producer_consumer_time = duration2.count();
    std::cout << "The producer consumer mode is " << serial_time/producer_consumer_time << " times faster\n";

    return 0;
}
