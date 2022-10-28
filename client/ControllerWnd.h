#pragma once
#include <QMainWindow>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>

class RenderWidget : public QOpenGLWidget,
                     protected QOpenGLFunctions
{
    Q_OBJECT
public:
    RenderWidget(QWidget *parent);
    ~RenderWidget();
    void UpdateData(QImage &_kImg);
protected slots:
    void ResizeTexture();
protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;
    void InitVbo();
    void InitTexture(QOpenGLTexture &_kTexture, int _nWidth, int _nHeight);
    void InitShaderProgram();
protected:
    int m_nMaxFrameWidth;
    int m_nMaxFrameHeight;
    int m_nTargetTextureWidth;
    int m_nTargetTextureHeight;
    QImage                  m_kFrameData;
    QOpenGLShader          *m_pkVertexShader;
    QOpenGLShader          *m_pkFragmentShader;
    QOpenGLShaderProgram   *m_pkShaderProgram;
    QOpenGLBuffer           m_kVertexBufferObj;
    QOpenGLTexture         *m_pkTexture;
};

class ControllerWnd : public QMainWindow
{
    Q_OBJECT
public:
    explicit ControllerWnd(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
signals:
    void MouseMoved(int _nX, int _nY);
    void MousePressed(Qt::MouseButton _eButton);
    void MouseReleased(Qt::MouseButton _eButton);
    void MouseDoubleClicked(Qt::MouseButton _eButton);
    void MouseWheel(QPoint _kAngleDelta);
    void KeyPressed(int _nKey);
    void KeyReleased(int _nKey);
public slots:
    void OnFrameUpdate(QImage _kImg);
    void UpdateFps();
protected:
    unsigned short m_nFrameCurrentSec;
    RenderWidget* m_pkRenderWidget;
};

