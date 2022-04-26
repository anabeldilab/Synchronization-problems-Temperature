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
    float serial_time = duration1.count();
    std::cout << duration1.count() << "\n";
    std::cout << "Done in serial mode\n";


    return 0;
}
