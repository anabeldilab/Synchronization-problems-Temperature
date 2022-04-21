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

class Statistics {
    // https://stackoverflow.com/questions/7988486/how-do-you-calculate-the-variance-median-and-standard-deviation-in-c-or-java/7988556#7988556
    QVector<float> data_;
    unsigned size_;

public:
    Statistics(QVector<float> data) {
        data_ = data;
        size_ = data.length();
    }

    Statistics(unsigned size) {
        data_.resize(size);
        size_ = data_.length();
    }

    double getMean() {
        double sum = 0.0;
        for(double a : data_)
            sum += a;
        return sum/size_;
    }

    double median() {
       //Arrays.sort(data);
        std::sort(data_.begin(),data_.end());
       if (data_.length() % 2 == 0)
          return (data_[(data_.length() / 2) - 1] + data_[data_.length() / 2]) / 2.0;
       return data_[data_.length() / 2];
    }
};

double getMean(QVector<float>* data) {
    return 0;
}

double median(QVector<float>* data) {
    return 1;
}

const unsigned TotalData = 60*60*24*365;        // sec * min * hour * days
const unsigned TotalBufferSize = 60*60*24*30;
QWaitCondition bufferNotEmpty;
QWaitCondition bufferNotFull;
QMutex mutex;
QVector<float> Buffer;
unsigned Index = 0;

void Producer() {
    for (unsigned int i = 0; i < TotalData; i++) {
        mutex.lock();
        if (Index >= TotalBufferSize) {
            bufferNotFull.wait(&mutex);
        }
        mutex.unlock();

        Buffer[Index] = random() % 50 + 50;

        mutex.lock();
        Index++;
        if (Index >= (24*3600))
            bufferNotEmpty.wakeAll();
        mutex.unlock();
    }
}


void Consumer() {
    for (unsigned int i = 0; i < TotalData; i++) {
        if (i % (24*3600) == 0) {
            mutex.lock();
               bufferNotEmpty.wait(&mutex);
            mutex.unlock();

            getMean(&Buffer);
            median(&Buffer);
            std::cout << i << " of " << TotalData << " ";
            std::cout << "average temperature " << getMean(&Buffer) << " ";
            std::cout << " with median " << median(&Buffer) << " ";
            std::cout << "Current index: " << Index << "\n";
            Index = Index - 24*3600;

            mutex.lock();
            bufferNotFull.wakeAll();
            mutex.unlock();
        }
    }
}


int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
/*
    auto start = std::chrono::high_resolution_clock::now();
    for(unsigned int i =0; i < TotalData;i++) {
        Buffer.push_back(random() % 50 + 50);
        if(i % (24*3600) == 0) {
            Statistics s(Buffer);
            s.getMean();
            s.median();
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
*/
    Buffer.resize(TotalBufferSize);

    auto start = std::chrono::high_resolution_clock::now();
    std::thread p(Producer),c(Consumer);
    p.join();
    c.join();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration2.count() << "\n";
    std::cout << "Done in producer/consumer mode\n";

    return 0;
}
