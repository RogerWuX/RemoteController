#include "ControllerWnd.h"

#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QMouseEvent>

#define FPS_UPDATE_INTERVAL_MSEC 1000
#define TEXTURE_SIZE_UPDATE_INTERVAL_MSEC 1000

RenderWidget::RenderWidget(QWidget *parent)
    :QOpenGLWidget(parent),
     m_nMaxFrameWidth(0),
     m_nMaxFrameHeight(0),
     m_nTargetTextureWidth(0),
     m_nTargetTextureHeight(0),
     m_pkTexture(nullptr),
     m_pkVertexShader(nullptr),
     m_pkFragmentShader(nullptr),
     m_pkShaderProgram(nullptr)
{
    setMouseTracking(true);
}

RenderWidget::~RenderWidget()
{
    makeCurrent();

    delete m_pkTexture;
    delete m_pkVertexShader;
    delete m_pkFragmentShader;
    delete m_pkShaderProgram;
    m_kVertexBufferObj.destroy();

    doneCurrent();
}

void RenderWidget::UpdateData(QImage &_kImg)
{
    m_nMaxFrameWidth = std::max(m_nMaxFrameWidth, _kImg.width());
    m_nMaxFrameHeight = std::max(m_nMaxFrameHeight, _kImg.height());

    m_kFrameData = _kImg;
    update();
}

// Resize texture size depend on max received frame size after last update
void RenderWidget::ResizeTexture()
{
    m_nTargetTextureWidth = std::min(width(),m_nMaxFrameWidth);
    m_nTargetTextureHeight = std::min(height(), m_nMaxFrameHeight);

    m_nMaxFrameWidth = 0;
    m_nMaxFrameHeight = 0;
}

void RenderWidget::initializeGL()
{
    initializeOpenGLFunctions();    

    InitVbo();    
    InitShaderProgram();

    m_pkShaderProgram->bindAttributeLocation("vertex", 0);
    m_pkShaderProgram->bindAttributeLocation("texCoord", 1);
    m_pkShaderProgram->bind();

    m_pkShaderProgram->enableAttributeArray(0);
    m_pkShaderProgram->enableAttributeArray(1);
    m_pkShaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 5*sizeof(GL_FLOAT));
    m_pkShaderProgram->setAttributeBuffer(1, GL_FLOAT, 3*sizeof(GL_FLOAT), 2, 5*sizeof(GL_FLOAT));

    m_pkShaderProgram->setUniformValue("texture",0);

    m_pkTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    InitTexture(*m_pkTexture, width(), height());

    QTimer *kResizeTextureTimer = new QTimer(this);
    QObject::connect(kResizeTextureTimer, &QTimer::timeout, this, &RenderWidget::ResizeTexture);
    kResizeTextureTimer->start(TEXTURE_SIZE_UPDATE_INTERVAL_MSEC);
}

void RenderWidget::resizeGL(int width, int height)
{
}

void RenderWidget::paintGL()
{
    if(m_nTargetTextureWidth && m_nTargetTextureHeight){
        delete m_pkTexture;
        m_pkTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);;
        InitTexture(*m_pkTexture, m_nTargetTextureWidth, m_nTargetTextureHeight);
        m_nTargetTextureWidth = 0;
        m_nTargetTextureHeight = 0;
    }

    if(!m_kFrameData.isNull()){
        QImage kFrameData = m_kFrameData.scaled(m_pkTexture->width(),m_pkTexture->height());
        m_pkTexture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, (void*)kFrameData.constBits());
    }

    glViewport(0,0,width(),height());
    m_pkTexture->bind();
    glDrawArrays(GL_TRIANGLE_FAN,0,4);    
}

void RenderWidget::InitVbo()
{
    const float afVertexCoord[4][3]= {{-1.0f,-1.0f,0.0f},{1.0f,-1.0f,0.0f},{1.0f,1.0f,0.0f},{-1.0f,1.0f,0.0f}};
    const float afTexCoord[4][2]={{0.0f,1.0f},{1.0f,1.0f},{1.0f,0.0f},{0.0f,0.0f}};

    GLfloat afBuffer[20];
    int j = -1;
    for(int i = 0; i < 4; ++i) {
       afBuffer[++j] = afVertexCoord[i][0];
       afBuffer[++j] = afVertexCoord[i][1];
       afBuffer[++j] = afVertexCoord[i][2];
       afBuffer[++j] = afTexCoord[i][0];
       afBuffer[++j] = afTexCoord[i][1];
    }

    m_kVertexBufferObj.create();
    m_kVertexBufferObj.bind();
    m_kVertexBufferObj.allocate(afBuffer, 20*sizeof(GL_FLOAT));
}

void RenderWidget::InitTexture(QOpenGLTexture &_kTexture, int _nWidth, int _nHeight)
{
    _kTexture.setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    _kTexture.setSize(_nWidth, _nHeight);
    _kTexture.setFormat(QOpenGLTexture::RGB8_UNorm);
    _kTexture.allocateStorage();
}

void RenderWidget::InitShaderProgram()
{
    // Direct send vertex position without matrix
    const char* pVertexShaderSrc =
    "attribute highp vec4 attrVertex;"
    "attribute mediump vec4 attrTextureCoord;"
    "varying mediump vec4 fragmentTextureCoord;"
    "void main (void)"
    "{"
    "   gl_Position = attrVertex;"
    "   fragmentTextureCoord = attrTextureCoord;"
    "}";
    m_pkVertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
    m_pkVertexShader->compileSourceCode(pVertexShaderSrc);

    const char* pFragmentShaderSrc =
    "uniform sampler2D smpTexture;"
    "varying mediump vec4 fragmentTextureCoord;"
    "void main(void)"
    "{"
    "   gl_FragColor = texture2D(smpTexture, fragmentTextureCoord.st);"
    "}";
    m_pkFragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
    m_pkFragmentShader->compileSourceCode(pFragmentShaderSrc);

    m_pkShaderProgram = new QOpenGLShaderProgram();
    m_pkShaderProgram->addShader(m_pkVertexShader);
    m_pkShaderProgram->addShader(m_pkFragmentShader);
    m_pkShaderProgram->link();
}

ControllerWnd::ControllerWnd(QWidget *parent) :
    QMainWindow(parent),
    m_nFrameCurrentSec(0)
{
    QScreen *pkPrimaryScreen = QApplication::primaryScreen();
    QSize kScreenSize = pkPrimaryScreen->size();
    setGeometry(0, 0, kScreenSize.width()/2, kScreenSize.height()/2);

    setMouseTracking(true);

    m_pkRenderWidget = new RenderWidget(this);
    m_pkRenderWidget->setGeometry(0,0,width(),height());
    m_pkRenderWidget->show();

    QTimer* kFpsUpdateTimer = new QTimer(this);
    QObject::connect(kFpsUpdateTimer,&QTimer::timeout,this,&ControllerWnd::UpdateFps);
    kFpsUpdateTimer->start(FPS_UPDATE_INTERVAL_MSEC);
}

void ControllerWnd::resizeEvent(QResizeEvent *event)
{
    m_pkRenderWidget->resize(width(),height());
}

void ControllerWnd::mouseMoveEvent(QMouseEvent *event)
{
    emit MouseMoved(event->x(), event->y());
}

void ControllerWnd::mousePressEvent(QMouseEvent *event)
{
    emit MousePressed(event->button());
}

void ControllerWnd::mouseReleaseEvent(QMouseEvent *event)
{
    emit MouseReleased(event->button());
}

void ControllerWnd::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit MouseDoubleClicked(event->button());
}

void ControllerWnd::wheelEvent(QWheelEvent *event)
{
    emit MouseWheel(event->angleDelta());
}

void ControllerWnd::keyPressEvent(QKeyEvent *event)
{
    emit KeyPressed(event->nativeVirtualKey());
}

void ControllerWnd::keyReleaseEvent(QKeyEvent *event)
{
    emit KeyReleased(event->nativeVirtualKey());
}

void ControllerWnd::OnFrameUpdate(QImage _kImg)
{    
    m_pkRenderWidget->UpdateData(_kImg);
    ++m_nFrameCurrentSec;
}

void ControllerWnd::UpdateFps()
{
    QString kFps = QString::number(m_nFrameCurrentSec);
    setWindowTitle(kFps);
    m_nFrameCurrentSec = 0;
}
