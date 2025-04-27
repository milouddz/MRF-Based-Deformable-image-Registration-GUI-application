#ifndef LONGTASKWORKER_H
#define LONGTASKWORKER_H

#include <QMainWindow>

class LongTaskWorker : public QMainWindow
{
    Q_OBJECT
public:
    explicit LongTaskWorker(QWidget *parent = nullptr);
public slots :
     void doLongTask();
signals:
    void progress(int value);
    void finished();
};

#endif // LONGTASKWORKER_H
