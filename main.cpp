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
        lock.lockForRead();
        if (Comparator < SampleSize)
           bufferNotEmpty.wait(&lock);
        lock.unlock();

        std::cout << counter << " of " << TotalData << " ";
        std::cout << "average temperature " << getMean(&Buffer, counter % TotalBufferSize) << " ";
        std::cout << " with median " << median(&Buffer, counter % TotalBufferSize) << " ";
        std::cout << "Current writer index: " << Comparator << "\n";
        counter += SampleSize*2;

        lock.lockForRead();
        Comparator = Comparator - SampleSize;
        bufferNotFull.wakeAll();
        lock.unlock();
    }
}


int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    Buffer.resize(TotalBufferSize);

    auto start3 = std::chrono::high_resolution_clock::now();
    std::thread w(Writer), r1(Reader, 0), r2(Reader, SampleSize);
    w.join();
    r1.join();
    r2.join();
    auto stop3 = std::chrono::high_resolution_clock::now();
    auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(stop3 - start3);
    std::cout << duration3.count() << "\n";
    std::cout << "Done in readers writer mode\n";

    return 0;
}