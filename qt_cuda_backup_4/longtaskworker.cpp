#include "longtaskworker.h"
#include <QThread>

LongTaskWorker::LongTaskWorker(QWidget *parent)
    : QMainWindow{parent}
{

}

void LongTaskWorker::doLongTask()
{
    for (int i = 0; i <= 10; ++i) {
                // Simulate a long-running task
                QThread::sleep(1); // This simulates taking time to complete the task
                emit progress(i * 10); // Emit progress signal
            }
            emit finished(); // Emit finished signal when done
}
