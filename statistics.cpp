#include "statistics.h"


Statistics::Statistics(QVector<float> data) {
    data_ = data;
    size_ = data.length();
}

Statistics::Statistics(unsigned size) {
    data_.resize(size);
    size_ = data_.length();
}

double Statistics::getMean() {
    double sum = 0.0;
    for(double a : data_)
        sum += a;
    return sum/size_;
}

double Statistics::median() {
   //Arrays.sort(data);
    std::sort(data_.begin(),data_.end());
   if (data_.length() % 2 == 0)
      return (data_[(data_.length() / 2) - 1] + data_[data_.length() / 2]) / 2.0;
   return data_[data_.length() / 2];
}
