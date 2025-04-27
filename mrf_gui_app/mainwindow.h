#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGLWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QColor>
#include <QTextBrowser>
#include <QString>
#include "string"
#include <QDebug>
#include <QThread>
#include "openglwidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QImage>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QTimer>

//QString fixed_image_path;
//QString mouving_image_path;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
extern "C" void squareArray(float* h_out, float* h_in, int size);

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include "shaders.h"
#include <QTimer>
#include <random>







class CustomOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    CustomOpenGLWidget(QWidget *parent = nullptr) : QOpenGLWidget(parent) {}

protected:
    void initializeGL() override {
            initializeOpenGLFunctions();
            glGenTextures(1, &texture); // Generate a texture ID
        }

        void resizeGL(int w, int h) override {
            width = w;
            height = h;
            glViewport(0, 0, w, h);
        }

    void paintGL() override {
        // Set up the texture
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                // Clear the color buffer
                glClear(GL_COLOR_BUFFER_BIT);

                // Draw a quad with the texture
                glBegin(GL_QUADS);
                    glTexCoord2f(0, 1); glVertex2f(-1.0f, -1.0f);
                    glTexCoord2f(1, 1); glVertex2f(1.0f, -1.0f);
                    glTexCoord2f(1, 0); glVertex2f(1.0f, 1.0f);
                    glTexCoord2f(0, 0); glVertex2f(-1.0f, 1.0f);
                glEnd();

                // Unbind texture
                glBindTexture(GL_TEXTURE_2D, 0);
    }
public slots:
    void changePixelColor(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
            // Check bounds
            if (x < 0 || x >= width || y < 0 || y >= height) return;

            int index = (y * width + x) * 3; // Calculate the index in the pixelData array
            pixelData[index + 0] = r; // Red channel
            pixelData[index + 1] = g; // Green channel
            pixelData[index + 2] = b; // Blue channel

            update(); // Request a repaint
        }
private:
    GLuint texture; // OpenGL texture ID
    unsigned char *pixelData; // Array to hold pixel data
    int width, height; // Size of the widget
};

class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    MyOpenGLWidget(QWidget *parent = nullptr) : QOpenGLWidget(parent), shaderProgram(nullptr), VAO(nullptr), VBO(nullptr), texture(nullptr) {


    }
    void updateImage() {

        QSize currentSize = this->size();
        //qDebug() << "Current size:" << currentSize.width() << "x" << currentSize.height();
        // Here you would grab or process your image.
        // For example, loading a new image file:
        resizeGL(400,400);
        QString imagePath = "/home/miloud/qt_cuda/image.jpg";
        //QImage image(imagePath); // Replace with your source of input
        //qDebug() << "Loading image from:" << imagePath;
        // Create a simple grayscale array (for example, a gradient)
           const int width = 400;
           const int height = 400;
           unsigned char grayscaleData[width * height];

           // Initialize random number generator for grayscale values
           std::random_device rd;
           std::mt19937 eng(rd());
           std::uniform_int_distribution<> distr(0, 255); // Range for grayscale
           // Fill the array with a gradient from black to white
           for (int y = 0; y < height; ++y) {
               for (int x = 0; x < width; ++x) {
                   grayscaleData[y * width + x] = static_cast<unsigned char>(distr(eng)); // Simple gradient
               }
           }
           // Create a QImage from the grayscale array
               QImage grayImage(grayscaleData, width, height, QImage::Format_Grayscale8);

        if (!QFile::exists(imagePath)) {
            qWarning("Image file does not exist: %s", qPrintable(imagePath));
            return;
        }
        if (!grayImage.isNull()) {
            if (texture) {
                delete texture; // Delete the old texture
            }
            // Convert image to texture format and create a new texture
            //QImage textureImage = grayImage.convertToFormat(QImage::Format_Grayscale8);
            if (grayImage.isNull()) {
                qWarning("Failed to convert image to RGBA format");
                return;
            }
            texture = new QOpenGLTexture(grayImage);
            update(); // Request a repaint
        } else {
            qWarning("Failed to load image");
        }

    }

    void updateImage_2();
    void updateImage_3();
    void updateImage_4();
    void start_update_image_2();
    void start_update_image_3();
    void start_update_image_4();

protected:
    void initializeGL() override;
    void paintGL() override {
        glClear(GL_COLOR_BUFFER_BIT);

            shaderProgram->bind(); // Use the shader program
            texture->bind(); // Bind the texture

            VAO->bind(); // Bind the VAO to render
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Draw the textured square

            VAO->release(); // Unbind VAO
            texture->release(); // Unbind texture
            shaderProgram->release(); // Unbind shader
    }

    void resizeGL(int w, int h) override {
        glViewport(0, 0, w, h);
    }

private:
    QOpenGLShaderProgram *shaderProgram;
    QOpenGLVertexArrayObject *VAO;
    QOpenGLBuffer *VBO;
    QOpenGLTexture *texture;
    QTimer *timer; // Timer for updating
    void loadShaders();




};

// Worker class that will do the long-running task

class Worker : public QObject {
    Q_OBJECT
public slots:
    void process_registration_2();


signals:
    void progress(const QString&);
    void finished();
    void warped_image_loaded();
    void difference_image_loaded();
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent= nullptr);
    ~MainWindow();
    QOpenGLWidget *openGLWidget; // Pointer to the OpenGL widget
    Worker* worker;
    float ***dataArray; // Point        for(int i=0;i<width;i++){
    int sizeX, sizeY, sizeZ; // Dimensions of the 3D array

    QThread *thread_1;    // Thread for performing the task
    void print_in_textBrowser(QString);
    void startLongTask();
    void startTask();
    void on_load_Fixed_image();
    void on_load_Moving_image();

public slots:
    void run_registration();
    void updateText(const QString&);
    void oncomboboxchanged();
    void label_warped_changed();
    void label_diffrence_changed();




signals:
    void taskFinished(); // Signal when task is finished
    void updateProgress(int value);
    void workDone(const QTextCursor); // Signal carrying QTextCursor
    void fixed_image_loaded();
    void moving_image_loaded();

protected:
    void initializeGL();
    void resizeGL(int w, int h);


private slots:
    void refreshData();
    void onChangeColorClick();

    void on_comboBox_2_currentTextChanged(const QString &arg1);

private:

    Ui::MainWindow *ui;
    QColor backgroundColor; // Variable to store background color
    CustomOpenGLWidget *customOpenGLWidget; // Pointer to your OpenGL widget
    MyOpenGLWidget *myopenglwidget_difference;
    MyOpenGLWidget *myopenglwidget_moving;

};


#endif // MAINWINDOW_H
