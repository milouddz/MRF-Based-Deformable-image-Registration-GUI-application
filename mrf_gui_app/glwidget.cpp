#include "glwidget.h"

GLWidget::GLWidget()
{

}

void GLWidget::initializeGL()
{
      glClearColor(0, 0, 1, 1); //define blue color as background color
}

void GLWidget::resizeGL(int w, int h)
{

}

void GLWidget::paintGL()
{
      glClear(GL_COLOR_BUFFER_BIT);
}
