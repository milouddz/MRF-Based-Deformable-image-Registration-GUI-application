#include "OpenGLWidget.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QDebug>

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
}

OpenGLWidget::~OpenGLWidget()
{
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions(); // Initialize OpenGL functions

    // Check OpenGL version
    const GLubyte* version = glGetString(GL_VERSION);
    qDebug() << "OpenGL Version: " << reinterpret_cast<const char*>(version); // Log version to the console

    // Enable depth test for 3D rendering
    glEnable(GL_DEPTH_TEST);

    // Define vertices for a simple cube
    GLfloat vertices[] = {
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f
    };


}

void OpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void OpenGLWidget::paintGL()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


}
