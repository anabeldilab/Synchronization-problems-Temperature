#include <iostream>
#include <QVector>
#include <math.h>

#ifndef STATISTICS_H
#define STATISTICS_H

class Statistics {
    // https://stackoverflow.com/questions/7988486/how-do-you-calculate-the-variance-median-and-standard-deviation-in-c-or-java/7988556#7988556
    QVector<float> data_;
    unsigned size_;

public:
    Statistics(QVector<float> data);
    Statistics(unsigned size);
    double getMean();
    double median();
};

#endif // STATISTICS_H
