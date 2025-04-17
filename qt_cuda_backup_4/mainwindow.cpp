#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <QProcess>
#include <QPushButton>
#include <iostream>
#include "GL/gl.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include "QGLWidget"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <GL/glu.h>
#include "openglwidget.h"
#include <QOpenGLFunctions>
#include <QOpenGLWindow>

#include <iostream>
#include <fstream>
#include <math.h>
#include <sys/time.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <map>
#include <functional>
#include <string.h>
#include <sstream>
#include <x86intrin.h>
#include <pthread.h>
#include <QThread>
#include <cstddef>
#include "zlib.h"
#include <sys/stat.h>
#include <GL/glu.h> // Ensure this is included for gluPerspective
#include "vertex_shader.h"
#include "fragment_shader.h"
#include "QPixmap"

using namespace std;
#include <chrono>
using namespace std::chrono;

//some global variables
int RAND_SAMPLES; //will all be set later (if needed)
int image_m; int image_n; int image_o; int image_d=1;
float SSD0=0.0; float SSD1=0.0; float SSD2=0.0; float distfx_global; float beta=1;
int image_max;int image_min;
int diff_min;int diff_max;
int warped_slice;
int diffrence_slice;
int slider_max;
//float SIGMA=8.0;
int qc=1;
QString view;

//struct for multi-threading of mind-calculation
struct mind_data{
    float* im1;
    float* d1;
    unsigned long* mindq;
    int qs;
    int ind_d1;
};

struct parameters{
    float alpha = 5.0; int levels; bool segment,affine,rigid;
    std::vector<int> grid_spacing; std::vector<int> search_radius;
    std::vector<int> quantisation;
    std::string fixed_file,moving_file,output_stem,moving_seg_file,affine_file,deformed_file;
};

#include "imageIOgzType.h"
#include "transformations.h"
#include "primsMST.h"
#include "regularisation.h"
#include "MINDSSCbox.h"
#include "dataCostD.h"
#include "parseArguments.h"
#include <QMatrix4x4>

parameters args;
float* im1; float* im1b;float *warped1;float* warped0;float *image_diffrence;
int M,N,O,P; //image dimensions
//==ALWAYS ALLOCATE MEMORY FOR HEADER ===/
char* header=new char[352];
void launch_registration();
#include "thread"
int opengl_width = 391;
int opengl_height = 441;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    // Create and set the OpenGL widget


    myopenglwidget_moving = new MyOpenGLWidget(ui->openGLWidget_warped);
    view = ui->comboBox_2->currentText();
    QSize currentSize = ui->centralwidget->size(); // Get the current size
    //qDebug() << "Current size:" << currentSize.width() << "x" << currentSize.height();
    connect(ui->comboBox_2,&QComboBox::currentTextChanged,this,&MainWindow::oncomboboxchanged);
    //connect(ui->horizontalSlider_diffrence,&QSlider::valueChanged,this,&MainWindow::label_diffrence_changed);
    myopenglwidget_difference = new MyOpenGLWidget(ui->openGLWidget_difference);
    //myopenglwidget_moving = new MyOpenGLWidget(ui->openGLWidget_warped);

    //connect(this, &MainWindow::fixed_image_loaded, myopenglwidget_fixed, &MyOpenGLWidget::start_update_image_2);
    //connect(this, &MainWindow::warped_image_loaded, myopenglwidget_moving, &MyOpenGLWidget::start_update_image_3);
    //connect(this, &MainWindow::moving_image_loaded, myopenglwidget_moving, &MyOpenGLWidget::start_update_image_3);


    connect(ui->computeButton, &QPushButton::clicked, this, &MainWindow::startTask);



    connect(ui->load_fixed_image, &QPushButton::clicked, this, &MainWindow::on_load_Fixed_image);
    connect(ui->load_mouving_image,&QPushButton::clicked, this, &MainWindow::on_load_Moving_image);


}


MainWindow::~MainWindow()
{
    delete ui;
}





void MainWindow::updateText(const QString &text)
{
    // Update the QTextBrowser with progress
    ui->textBrowser_output->append(text);
}

void MainWindow::oncomboboxchanged()
{
    view = ui->comboBox_2->currentText();

    if(view == "Axial"){
        ui->horizontalSlider_diffrence->setMinimum(0);
        ui->horizontalSlider_diffrence->setMaximum(image_n);
     }
    else if(view == "Sagitall"){
        ui->horizontalSlider_diffrence->setMinimum(0);
        ui->horizontalSlider_diffrence->setMaximum(image_m);
     }
    else if(view == "Coronal"){
        ui->horizontalSlider_diffrence->setMinimum(0);
        ui->horizontalSlider_diffrence->setMaximum(image_o);
     }

    qDebug() << view ;
}

void MainWindow::label_warped_changed()
{
    ui->label_warped->setText(QString::number(ui->horizontalSlider_warped->value()));
    warped_slice = ui->horizontalSlider_warped->value();

}

void MainWindow::label_diffrence_changed()
{

    ui->label_diffrence->setText(QString::number(ui->horizontalSlider_diffrence->value()));
    diffrence_slice = ui->horizontalSlider_diffrence->value();
}


void MainWindow::print_in_textBrowser(QString text)
{
    ui->textBrowser_output->append(text);
}

void MainWindow::startTask()
{





    //connect(this, &MainWindow::fixed_image_loaded, myopenglwidget_fixed, &MyOpenGLWidget::start_update_image_2);

    QThread *thread = new QThread; // Create a new thread
    worker = new Worker; // Create worker object

    // Move the worker to the new thread
    worker->moveToThread(thread);
    connect(worker, &Worker::warped_image_loaded, myopenglwidget_moving, &MyOpenGLWidget::start_update_image_3);
    connect(worker, &Worker::difference_image_loaded, myopenglwidget_difference, &MyOpenGLWidget::start_update_image_4);
    // Connect signals and slots
    connect(thread, &QThread::started, worker, &Worker::process_registration_2);
    connect(worker, &Worker::progress, this, &MainWindow::updateText);
    connect(worker, &Worker::finished, thread, &QThread::quit); // Quit thread when done
    connect(worker, &Worker::finished, worker, &QObject::deleteLater); // Clean up worker
    connect(thread, &QThread::finished, thread, &QThread::deleteLater); // Clean up thread

    thread->start(); // Start the thread
}


void MainWindow::on_load_Fixed_image()
{
    // Open a file dialog to select a file
        QString filePath = QFileDialog::getOpenFileName(this, tr("Select File"), "", tr("All Files (*)"));

        // Check if a file was selected
        if (!filePath.isEmpty()) {
            // Print the file path (can be used as needed)
            qDebug() << "Selected file path:" << filePath;

            // Optional: Show the file path in a message box
            QMessageBox::information(this, tr("File Selected"), tr("You selected:\n%1").arg(filePath));
        } else {
            // Inform the user that no file was selected
            QMessageBox::warning(this, tr("No File Selected"), tr("Please select a file."));
        }

    //fixed_image_path = filePath;
    //qDebug() << fixed_image_path ;
        args.fixed_file = filePath.toStdString();
        size_t split_fixed=args.fixed_file.find_last_of("/\\");
        if(split_fixed==string::npos){
            split_fixed=-1;
        }
        readNifti(args.fixed_file,im1b,header,M,N,O,P);
        image_m=M; image_n=N; image_o=O;
        //qDebug() << M << " " << N << " " << O;
        ui->textBrowser_output->append("fixed image loaded : " + filePath);
        ui->textBrowser_output->append("image size : " + QString::number(M) + " " + QString::number(N) + " " + QString::number(O));

        QSize currentSize = ui->centralwidget->size(); // Get the current size
        //qDebug() << "Current size:" << currentSize.width() << "x" << currentSize.height();
        ui->horizontalSlider_diffrence->setMinimum(0);
        ui->horizontalSlider_diffrence->setMaximum(image_n);
        ui->horizontalSlider_warped->setMinimum(0);
        ui->horizontalSlider_warped->setMaximum(image_n);

        connect(ui->horizontalSlider_diffrence,&QSlider::valueChanged,this,&MainWindow::label_diffrence_changed);
        connect(ui->horizontalSlider_warped,&QSlider::valueChanged,this,&MainWindow::label_warped_changed);

        //emit fixed_image_loaded();




}


void MainWindow::on_load_Moving_image()
{
    // Open a file dialog to select a file
        QString filePath = QFileDialog::getOpenFileName(this, tr("Select File"), "", tr("All Files (*)"));

        // Check if a file was selected
        if (!filePath.isEmpty()) {
            // Print the file path (can be used as needed)
            qDebug() << "Selected file path:" << filePath;

            // Optional: Show the file path in a message box
            QMessageBox::information(this, tr("File Selected"), tr("You selected:\n%1").arg(filePath));
        } else {
            // Inform the user that no file was selected
            QMessageBox::warning(this, tr("No File Selected"), tr("Please select a file."));
        }

    //fixed_image_path = filePath;
    //qDebug() << fixed_image_path ;
        args.moving_file = filePath.toStdString();
        size_t split_moving=args.moving_file.find_last_of("/\\");
        if(split_moving==string::npos){
            split_moving=-1;
        }
        readNifti(args.moving_file,im1,header,M,N,O,P);
        //image_m=M; image_n=N; image_o=O;
        //qDebug() << M << " " << N << " " << O;
        ui->textBrowser_output->append("moving image loaded : " + filePath);

        emit moving_image_loaded();

}




void MainWindow::run_registration()
{

    QThread *thread = new QThread;


}



void MainWindow::initializeGL()
{

    //initializeOpenGLFunctions();

}


void MainWindow::refreshData()
{

}


void MainWindow::onChangeColorClick()
{

}





void Worker::process_registration_2()
{
    //PARSE INPUT ARGUMENTS

       char * argv[] = {"arg1","-F","arg3","-M","arg5"};
       parameters args;
       //defaults
       args.grid_spacing={8,7,6,5,4};
       args.search_radius={8,7,6,5,4};
       args.quantisation={5,4,3,2,1};
       args.levels=5;
       int argc = 5;
       argv[2] = &args.fixed_file[0];
       argv[4] = &args.moving_file[0];
       parseCommandLine(args, argc, argv);

       //cout<<"Starting registration of "<<args.fixed_file.substr(split_fixed+1)<<" and "<<args.moving_file.substr(split_moving+1)<<"\n";
       //cout<<"=============================================================\n";

       string outputflow;
       outputflow.append(args.output_stem);
       outputflow.append("_displacements.dat");
       string outputfile;
       outputfile.append(args.output_stem);
       outputfile.append("_deformed.nii.gz");


       //READ IMAGES and INITIALISE ARRAYS

       timeval time1,time2,time1a,time2a;

       RAND_SAMPLES=1; //fixed/efficient random sampling strategy

       //float* im1; float* im1b;

       int M,N,O,P; //image dimensions

       //==ALWAYS ALLOCATE MEMORY FOR HEADER ===/
       //char* header=new char[352];


       //image_m=M; image_n=N; image_o=O;





       int m=image_m; int n=image_n; int o=image_o; int sz=m*n*o;
       image_max = 0;
       image_min = 10000;

       //assume we are working with CT scans (add 1024 HU)
       float thresholdF=-1024; float thresholdM=-1024;

       for(int i=0;i<sz;i++){
           im1b[i]-=thresholdF;
           im1[i]-=thresholdM;
           if(image_max<im1[i]) image_max = im1[i] ;
       }
        qDebug() << "max and min " << image_max << "  " << image_min;
       warped1=new float[m*n*o];

       //READ AFFINE MATRIX from linearBCV if provided (else start from identity)

       float* X=new float[16];
        QString pass_text;
       if(args.affine){
           size_t split_affine=args.affine_file.find_last_of("/\\");
           if(split_affine==string::npos){
               split_affine=-1;
           }

           //cout<<"Reading affine matrix file: "<<args.affine_file.substr(split_affine+1)<<"\n";
           ifstream matfile;
           matfile.open(args.affine_file);
           for(int i=0;i<4;i++){
               string line;
               getline(matfile,line);
               sscanf(line.c_str(),"%f  %f  %f  %f",&X[i],&X[i+4],&X[i+8],&X[i+12]);
           }
           matfile.close();


       }
       else{
           qDebug() <<"Starting with identity transform.\n";
           fill(X,X+16,0.0f);
           X[0]=1.0f; X[1+4]=1.0f; X[2+8]=1.0f; X[3+12]=1.0f;
       }

       for(int i=0;i<4;i++){
           //printf("%+4.3f | %+4.3f | %+4.3f | %+4.3f \n",X[i],X[i+4],X[i+8],X[i+12]);//X[i],X[i+4],X[i+8],X[i+12]);
           pass_text = (QString::number(X[i]) + "+4.3f | "+ QString::number(X[i+4])+ "4.3f | "+ QString::number(X[i+8])+"4.3f | " +QString::number(X[i+12])+"4.3f");//X[i],X[i+4],X[i+8],X[i+12]);
           emit progress(pass_text);
       }

       //PATCH-RADIUS FOR MIND/SSC DESCRIPTORS

       vector<int> mind_step;
       for(int i=0;i<args.quantisation.size();i++){
           mind_step.push_back(floor(0.5f*(float)args.quantisation[i]+1.0f));
       }
       pass_text = "MIND STEPS, "+ QString::number(mind_step[0]) + " " + QString::number(mind_step[1])+ " " +QString::number(mind_step[2])+ " " +QString::number(mind_step[3])+ " " +QString::number(mind_step[4]);
        emit progress(pass_text);


       int step1; int hw1; float quant1;

       //set initial flow-fields to 0; i indicates backward (inverse) transform
       //u is in x-direction (2nd dimension), v in y-direction (1st dim) and w in z-direction (3rd dim)
       float* ux=new float[sz]; float* vx=new float[sz]; float* wx=new float[sz];
        image_diffrence = new float[m*n*o];
       for(int i=0;i<sz;i++){
           ux[i]=0.0; vx[i]=0.0; wx[i]=0.0;
           warped1[i] = 0.0f;
           image_diffrence[i]=0.0f;
       }
       emit difference_image_loaded();
       emit warped_image_loaded();
       int m2,n2,o2,sz2;
       int m1,n1,o1,sz1;
       m2=m/args.grid_spacing[0]; n2=n/args.grid_spacing[0]; o2=o/args.grid_spacing[0]; sz2=m2*n2*o2;
       float* u1=new float[sz2]; float* v1=new float[sz2]; float* w1=new float[sz2];
       float* u1i=new float[sz2]; float* v1i=new float[sz2]; float* w1i=new float[sz2];
       for(int i=0;i<sz2;i++){
           u1[i]=0.0; v1[i]=0.0; w1[i]=0.0;
           u1i[i]=0.0; v1i[i]=0.0; w1i[i]=0.0;
       }
       diff_min = 10000;
       diff_max = -10000;
       for(int i=0;i<sz;i++){
          image_diffrence[i] = im1b[i] - im1[i];
          if(diff_min>image_diffrence[i])diff_min=image_diffrence[i];
          if(diff_max<image_diffrence[i])diff_max=image_diffrence[i];
        }
       warped0=new float[m*n*o];
       warpAffine(warped0,im1,im1b,X,ux,vx,wx);


       uint64_t* im1_mind=new uint64_t[m*n*o];
       uint64_t* im1b_mind=new uint64_t[m*n*o];
       uint64_t* warped_mind=new uint64_t[m*n*o];

       gettimeofday(&time1a, NULL);
       float timeDataSmooth=0;
       //==========================================================================================
       //==========================================================================================

       for(int level=0;level<args.levels;level++){
           quant1=args.quantisation[level];

           float prev=mind_step[max(level-1,0)];//max(min(label_quant[max(level-1,0)],2.0f),1.0f);
           float curr=mind_step[level];//max(min(label_quant[level],2.0f),1.0f);

           float timeMIND=0; float timeSmooth=0; float timeData=0; float timeTrans=0;

           if(level==0|prev!=curr){
               gettimeofday(&time1, NULL);
               descriptor(im1_mind,warped0,m,n,o,mind_step[level]);//im1 affine
               descriptor(im1b_mind,im1b,m,n,o,mind_step[level]);
               gettimeofday(&time2, NULL);
               timeMIND+=time2.tv_sec+time2.tv_usec/1e6-(time1.tv_sec+time1.tv_usec/1e6);
           }

           step1=args.grid_spacing[level];
           hw1=args.search_radius[level];

           int len3=pow(hw1*2+1,3);
           m1=m/step1; n1=n/step1; o1=o/step1; sz1=m1*n1*o1;

           float* costall=new float[sz1*len3];
           float* u0=new float[sz1]; float* v0=new float[sz1]; float* w0=new float[sz1];
           int* ordered=new int[sz1]; int* parents=new int[sz1]; float* edgemst=new float[sz1];

           cout<<"==========================================================\n";
           cout<<"Level "<<level<<" grid="<<step1<<" with sizes: "<<m1<<"x"<<n1<<"x"<<o1<<" hw="<<hw1<<" quant="<<quant1<<"\n";
           cout<<"==========================================================\n";

           //FULL-REGISTRATION FORWARDS
           gettimeofday(&time1, NULL);
           upsampleDeformationsCL(u0,v0,w0,u1,v1,w1,m1,n1,o1,m2,n2,o2);
           upsampleDeformationsCL(ux,vx,wx,u0,v0,w0,m,n,o,m1,n1,o1);
           //float dist=landmarkDistance(ux,vx,wx,m,n,o,distsmm,casenum);
           warpAffine(warped1,im1,im1b,X,ux,vx,wx);

           u1=new float[sz1]; v1=new float[sz1]; w1=new float[sz1];
           gettimeofday(&time2, NULL);
           timeTrans+=time2.tv_sec+time2.tv_usec/1e6-(time1.tv_sec+time1.tv_usec/1e6);
           cout<<"T"<<flush;
           gettimeofday(&time1, NULL);
           descriptor(warped_mind,warped1,m,n,o,mind_step[level]);

           gettimeofday(&time2, NULL);
           timeMIND+=time2.tv_sec+time2.tv_usec/1e6-(time1.tv_sec+time1.tv_usec/1e6);
           cout<<"M"<<flush;
           gettimeofday(&time1, NULL);
           dataCostCL((unsigned long*)im1b_mind,(unsigned long*)warped_mind,costall,m,n,o,len3,step1,hw1,quant1,args.alpha,RAND_SAMPLES);
           gettimeofday(&time2, NULL);

           timeData+=time2.tv_sec+time2.tv_usec/1e6-(time1.tv_sec+time1.tv_usec/1e6);
           cout<<"D"<<flush;
           gettimeofday(&time1, NULL);
           primsGraph(im1b,ordered,parents,edgemst,step1,m,n,o);
           regularisationCL(costall,u0,v0,w0,u1,v1,w1,hw1,step1,quant1,ordered,parents,edgemst);
           gettimeofday(&time2, NULL);
           timeSmooth+=time2.tv_sec+time2.tv_usec/1e6-(time1.tv_sec+time1.tv_usec/1e6);
           cout<<"S"<<flush;

           //FULL-REGISTRATION BACKWARDS
           gettimeofday(&time1, NULL);
           upsampleDeformationsCL(u0,v0,w0,u1i,v1i,w1i,m1,n1,o1,m2,n2,o2);
           upsampleDeformationsCL(ux,vx,wx,u0,v0,w0,m,n,o,m1,n1,o1);
           warpImageCL(warped1,im1b,warped0,ux,vx,wx);
           diff_min = 10000;
           diff_max = -10000;

           for(int i=0;i<sz;i++){
              image_diffrence[i] = im1b[i] - warped1[i];
              if(diff_min>image_diffrence[i])diff_min=image_diffrence[i];
              if(diff_max<image_diffrence[i])diff_max=image_diffrence[i];
            }
           u1i=new float[sz1]; v1i=new float[sz1]; w1i=new float[sz1];
           gettimeofday(&time2, NULL);
           timeTrans+=time2.tv_sec+time2.tv_usec/1e6-(time1.tv_sec+time1.tv_usec/1e6);
           cout<<"T"<<flush;
           gettimeofday(&time1, NULL);
           descriptor(warped_mind,warped1,m,n,o,mind_step[level]);

           gettimeofday(&time2, NULL);
           timeMIND+=time2.tv_sec+time2.tv_usec/1e6-(time1.tv_sec+time1.tv_usec/1e6);
           cout<<"M"<<flush;
           gettimeofday(&time1, NULL);
           dataCostCL((unsigned long*)im1_mind,(unsigned long*)warped_mind,costall,m,n,o,len3,step1,hw1,quant1,args.alpha,RAND_SAMPLES);
           gettimeofday(&time2, NULL);
           timeData+=time2.tv_sec+time2.tv_usec/1e6-(time1.tv_sec+time1.tv_usec/1e6);
           cout<<"D"<<flush;
           gettimeofday(&time1, NULL);
           primsGraph(warped0,ordered,parents,edgemst,step1,m,n,o);
           regularisationCL(costall,u0,v0,w0,u1i,v1i,w1i,hw1,step1,quant1,ordered,parents,edgemst);
           gettimeofday(&time2, NULL);
           timeSmooth+=time2.tv_sec+time2.tv_usec/1e6-(time1.tv_sec+time1.tv_usec/1e6);
           cout<<"S"<<flush;

           cout<<"\nTime: MIND="<<timeMIND<<", data="<<timeData<<", MST-reg="<<timeSmooth<<", transf.="<<timeTrans<<"\n speed="<<2.0*(float)sz1*(float)len3/(timeData+timeSmooth)<<" dof/s\n";


           gettimeofday(&time1, NULL);
           consistentMappingCL(u1,v1,w1,u1i,v1i,w1i,m1,n1,o1,step1);
           gettimeofday(&time2, NULL);
           float timeMapping=time2.tv_sec+time2.tv_usec/1e6-(time1.tv_sec+time1.tv_usec/1e6);

           //cout<<"Time consistentMapping: "<<timeMapping<<"  \n";

           //upsample deformations from grid-resolution to high-resolution (trilinear=1st-order spline)
           float jac=jacobian(u1,v1,w1,m1,n1,o1,step1);

           cout<<"SSD before registration: "<<SSD0<<" and after "<<SSD1<<"\n";
           m2=m1; n2=n1; o2=o1;
           cout<<"\n";

           delete[] u0; delete[] v0; delete[] w0;
           delete[] costall;

           delete[] parents; delete[] ordered;


       }
       delete[] im1_mind;
       delete[] im1b_mind;
       //==========================================================================================
       //==========================================================================================

       gettimeofday(&time2a, NULL);
       float timeALL=time2a.tv_sec+time2a.tv_usec/1e6-(time1a.tv_sec+time1a.tv_usec/1e6);

       upsampleDeformationsCL(ux,vx,wx,u1,v1,w1,m,n,o,m1,n1,o1);

       float* flow=new float[sz1*3];
       for(int i=0;i<sz1;i++){
           flow[i]=u1[i]; flow[i+sz1]=v1[i]; flow[i+sz1*2]=w1[i];
           //flow[i+sz1*3]=u1i[i]; flow[i+sz1*4]=v1i[i]; flow[i+sz1*5]=w1i[i];

       }

       //WRITE OUTPUT DISPLACEMENT FIELD AND IMAGE
       writeOutput(flow,outputflow.c_str(),sz1*3);
       warpAffine(warped1,im1,im1b,X,ux,vx,wx);
       for(int i=0;i<sz;i++){
            image_diffrence[i] = im1b[i] - warped1[i];
       }
       /*
       for(int i=0;i<sz;i++){
          warped1[i]+=thresholdM;
       }
        */
       gzWriteNifti(outputfile,warped1,header,m,n,o,1);

       pass_text = "SSD before registration: " + QString::number(SSD0) + " and after "+ QString::number(SSD1);
        emit progress(pass_text);
       // if SEGMENTATION of moving image is provided APPLY SAME TRANSFORM
       if(args.segment){
           short* seg2;
           readNifti(args.moving_seg_file,seg2,header,M,N,O,P);

           short* segw=new short[sz];
           fill(segw,segw+sz,0);

           warpAffineS(segw,seg2,X,ux,vx,wx);


           string outputseg;
           outputseg.append(args.output_stem);
           outputseg.append("_deformed_seg.nii.gz");
           cout<<"outputseg "<<outputseg<<"\n";



           gzWriteSegment(outputseg,segw,header,m,n,o,1);
       }

       //pass_text = "Finished. Total time: " +QString::number(timeALL) + " sec. (" + QString::number(timeDataSmooth) + " sec. for MIND+data+reg+trans)";
        //emit progress(pass_text);

}


void MyOpenGLWidget::updateImage_2()
{





}

void MyOpenGLWidget::updateImage_4()
{

    int width;
    int height;

    QSize currentSize = this->size();
    if(view == "Axial"){

        width = image_m;
        height = image_o;
        this->resize(width*2,height*2);
        unsigned char grayscaleData[width * height];
        int normalization_number =(diff_max+abs(diff_min))/255;
        // Initialize random number generator for grayscale values
        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distr(0, 255); // Range for grayscale
        int j=diffrence_slice;

        for (int k = 0; k < height; ++k) {
            for (int i = 0; i < width; ++i) {

                grayscaleData[k * width + i] = static_cast<unsigned char>((image_diffrence[i+j*image_m+k*image_m*image_n]+abs(diff_min))/normalization_number); // Simple gradient
            }
        }
        // Create a QImage from the grayscale array
        QImage grayImage(grayscaleData, width, height, QImage::Format_Grayscale8);


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
         texture = new QOpenGLTexture(grayImage.mirrored());
         update(); // Request a repaint
     } else {
         qWarning("Failed to load image");
     }
    }
    else if(view == "Coronal"){

        width = image_m;
        height = image_n;
        this->resize(width*2,height*2);
        unsigned char grayscaleData[width * height];
        int normalization_number =(diff_max+abs(diff_min))/255;
        // Initialize random number generator for grayscale values
        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distr(0, 255); // Range for grayscale
        int k=diffrence_slice;

        for (int j = 0; j < height; ++j) {
            for (int i = 0; i < width; ++i) {

                grayscaleData[j * width + i] = static_cast<unsigned char>((image_diffrence[i+j*image_m+k*image_m*image_n]+abs(diff_min))/normalization_number); // Simple gradient
            }
        }
        // Create a QImage from the grayscale array
        QImage grayImage(grayscaleData, width, height, QImage::Format_Grayscale8);


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
         texture = new QOpenGLTexture(grayImage.mirrored());
         update(); // Request a repaint
     } else {
         qWarning("Failed to load image");
     }
    }
    else if(view == "Sagitall"){

        width = image_n;
        height = image_o;
        this->resize(width*2,height*2);
        unsigned char grayscaleData[width * height];
        int normalization_number =(diff_max+abs(diff_min))/255;
        // Initialize random number generator for grayscale values
        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distr(0, 255); // Range for grayscale
        int i=diffrence_slice;

        for (int k = 0; k < height; ++k) {
            for (int j = 0; j < width; ++j) {

                grayscaleData[k * width + j] = static_cast<unsigned char>((image_diffrence[i+j*image_m+k*image_m*image_n]+abs(diff_min))/normalization_number); // Simple gradient
            }
        }
        // Create a QImage from the grayscale array
        QImage grayImage(grayscaleData, width, height, QImage::Format_Grayscale8);


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
         texture = new QOpenGLTexture(grayImage.mirrored());
         update(); // Request a repaint
     } else {
         qWarning("Failed to load image");
     }
    }


}

void MyOpenGLWidget::updateImage_3()
{


    int width;
    int height;

    QSize currentSize = this->size();
    if(view == "Axial"){

        width = image_m;
        height = image_o;
        this->resize(width*2,height*2);
        unsigned char grayscaleData[width * height];
        int normalization_number =(image_max)/255;
        // Initialize random number generator for grayscale values
        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distr(0, 255); // Range for grayscale
        int j=warped_slice;

        for (int k = 0; k < height; ++k) {
            for (int i = 0; i < width; ++i) {

                grayscaleData[k * width + i] = static_cast<unsigned char>(warped1[i+j*image_m+k*image_m*image_n]/normalization_number); // Simple gradient
            }
        }
        // Create a QImage from the grayscale array
        QImage grayImage(grayscaleData, width, height, QImage::Format_Grayscale8);


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
         texture = new QOpenGLTexture(grayImage.mirrored());
         update(); // Request a repaint
     } else {
         qWarning("Failed to load image");
     }
    }
    else if(view == "Coronal"){

        width = image_m;
        height = image_n;
        this->resize(width*2,height*2);
        unsigned char grayscaleData[width * height];
        int normalization_number =(image_max)/255;
        // Initialize random number generator for grayscale values
        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distr(0, 255); // Range for grayscale
        int k=warped_slice;

        for (int j = 0; j < height; ++j) {
            for (int i = 0; i < width; ++i) {

                grayscaleData[j * width + i] = static_cast<unsigned char>(warped1[i+j*image_m+k*image_m*image_n]/normalization_number); // Simple gradient
            }
        }
        // Create a QImage from the grayscale array
        QImage grayImage(grayscaleData, width, height, QImage::Format_Grayscale8);


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
         texture = new QOpenGLTexture(grayImage.mirrored());
         update(); // Request a repaint
     } else {
         qWarning("Failed to load image");
     }
    }
    else if(view == "Sagitall"){

        width = image_n;
        height = image_o;
        this->resize(width*2,height*2);
        unsigned char grayscaleData[width * height];
        int normalization_number =(image_max)/255;
        // Initialize random number generator for grayscale values
        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distr(0, 255); // Range for grayscale
        int i=warped_slice;

        for (int k = 0; k < height; ++k) {
            for (int j = 0; j < width; ++j) {

                grayscaleData[k * width + j] = static_cast<unsigned char>(warped1[i+j*image_m+k*image_m*image_n]/normalization_number); // Simple gradient
            }
        }
        // Create a QImage from the grayscale array
        QImage grayImage(grayscaleData, width, height, QImage::Format_Grayscale8);


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
         texture = new QOpenGLTexture(grayImage.mirrored());
         update(); // Request a repaint
     } else {
         qWarning("Failed to load image");
     }
    }



}

void MyOpenGLWidget::start_update_image_2()
{

            // Setup a timer to update the OpenGL widget regularly
                timer = new QTimer(this);
                connect(timer, &QTimer::timeout, this, &MyOpenGLWidget::updateImage_2); // Connect timer to update function
                timer->start(100); // Update every 100 milliseconds (adjust as needed)
}

void MyOpenGLWidget::start_update_image_3()
{

            // Setup a timer to update the OpenGL widget regularly
                timer = new QTimer(this);
                connect(timer, &QTimer::timeout, this, &MyOpenGLWidget::updateImage_3); // Connect timer to update function
                timer->start(100); // Update every 100 milliseconds (adjust as needed)
}

void MyOpenGLWidget::start_update_image_4()
{

            // Setup a timer to update the OpenGL widget regularly
                timer = new QTimer(this);
                connect(timer, &QTimer::timeout, this, &MyOpenGLWidget::updateImage_4); // Connect timer to update function
                timer->start(100); // Update every 100 milliseconds (adjust as needed)
}


void MyOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

        loadShaders(); // Method to load shaders

        // Vertex data
        // Setup VAO and VBO
        float vertices[] = {
            // Position        // Texture Coords
            -1.0f,  1.0f,    0.0f, 0.0f, // Top-left (texture should sample from bottom)
             1.0f,  1.0f,    1.0f, 0.0f, // Top-right (texture should sample from bottom)
             1.0f, -1.0f,    1.0f, 1.0f, // Bottom-right
            -1.0f, -1.0f,    0.0f, 1.0f  // Bottom-left
        };

        // Setup VAO and VBO
        VAO = new QOpenGLVertexArrayObject();
        VAO->create();
        VAO->bind();

        VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        VBO->create();
        VBO->bind();
        VBO->allocate(vertices, sizeof(vertices));

        // Set vertex attributes
        shaderProgram->enableAttributeArray(0); // Position
        shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2, 4 * sizeof(float));
        shaderProgram->enableAttributeArray(1); // Texture Coordinates
        shaderProgram->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(float), 2, 4 * sizeof(float));

        VBO->release();
        VAO->release();

}

void MyOpenGLWidget::loadShaders()
{

    shaderProgram = new QOpenGLShaderProgram();

        // Load the vertex shader from the string constant
        if (!shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) {
            qFatal("Failed to compile vertex shader: %s", qPrintable(shaderProgram->log()));
        }

        // Load the fragment shader from the string constant
        if (!shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) {
            qFatal("Failed to compile fragment shader: %s", qPrintable(shaderProgram->log()));
        }

        // Link the shader program
        if (!shaderProgram->link()) {
            qFatal("Failed to link shader program: %s", qPrintable(shaderProgram->log()));
        }


        // Load the texture (the same as before)
        QImage image("/home/miloud/qt_cuda/image.jpeg"); // Update the image path as necessary
        if (image.isNull()) {
            qWarning("Failed to load image: /home/your_username/qt_cuda/your_image.jpg");
        } else {
            texture = new QOpenGLTexture(image.convertToFormat(QImage::Format_RGBA8888));
        }


}




void MainWindow::on_comboBox_2_currentTextChanged(const QString &arg1)
{
    view = arg1;
    //qDebug() << view ;
}


#include "mainwindow.moc" // Include this at the end of your .cpp file that contains Q_OBJECT




