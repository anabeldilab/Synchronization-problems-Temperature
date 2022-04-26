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
        mutex.lock();
        if (Comparator < SampleSize)
           bufferNotEmpty.wait(&mutex);
        mutex.unlock();

        std::cout << counter << " of " << TotalData << " ";
        std::cout << "average temperature " << getMean(&Buffer, counter % TotalBufferSize) << " ";
        std::cout << " with median " << median(&Buffer, counter % TotalBufferSize) << " ";
        std::cout << "Current producer index: " << Comparator << "\n";

        mutex.lock();
        Comparator = Comparator - SampleSize;
        bufferNotFull.wakeAll();
        mutex.unlock();

        counter += SampleSize*2;
    }
}


int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    Buffer.resize(TotalBufferSize);

    auto start2 = std::chrono::high_resolution_clock::now();
    std::thread p(Producer),c1(Consumer, 0), c2(Consumer, SampleSize);
    p.join();
    c1.join();
    c2.join();
    auto stop2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(stop2 - start2);
    std::cout << duration2.count() << "\n";
    std::cout << "Done in producer/consumer mode\n";

    return 0;
}
