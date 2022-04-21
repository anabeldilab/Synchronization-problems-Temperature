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

double getMean(QVector<float>* data, unsigned size) {
    double sum = 0.0;
    for(double a : *data)
        sum += a;
    return sum/size;
}

double median(QVector<float>* data, unsigned size) {
   //Arrays.sort(data);
    std::sort((*data).begin(),(*data).end());
   if ((*data).length() % 2 == 0)
      return ((*data)[((*data).length() / 2) - 1] + (*data)[(*data).length() / 2]) / 2.0;
   return (*data)[(*data).length() / 2];
}

const unsigned TotalData = 60*60*24*365;        // sec * min * hour * days
const unsigned TotalBufferSize = 60*60*24*30;
QWaitCondition bufferNotEmpty;
QWaitCondition bufferNotFull;
QMutex mutex;
QVector<float> Buffer;

void Producer() {
    for (unsigned int i = 0; i < TotalData; i++) {
        mutex.lock();
        if (Buffer.size() == TotalBufferSize) {
            bufferNotFull.wait(&mutex);
        }
        mutex.unlock();

        Buffer.push_back(random() % 50 + 50);

        mutex.lock();
        if (Buffer.size() == 24*3600)
            bufferNotEmpty.wakeAll();
        mutex.unlock();
    }
}


void Consumer() {
    for (unsigned int i =0; i < TotalData; i++) {

        mutex.lock();
        if (Buffer.size() < 24*3600) {
            bufferNotEmpty.wait(&mutex);
        }
        mutex.unlock();

        if (i % 24*3600 == 0) {
            s.getMean();
            s.median();
            std::cout << i << " of " << TotalData << " ";
            std::cout << "average temperature " << s.getMean() << " ";
            std::cout << " with median " << s.median() << " ";
            std::cout << "Current C buffer size: " << Buffer.size() << "\n";

            mutex.lock();
            Buffer.erase(Buffer.constBegin(), Buffer.constEnd()); // borra todo el buffer
            //for(unsigned int i = 0; i < 24*3600; i++)
            //    Buffer.pop_front();
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
