#pragma once

#include <QMediaPlayer>
#include <QVideoSink>
#include <QVideoFrame>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QApplication>
#include <QOpenGLWidget>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QCryptographicHash>
#include <QPainter>
#include <QWidget>

class VideoFrameCapture : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit VideoFrameCapture(QMediaPlayer *player, QWidget *parent = nullptr)
        : QOpenGLWidget(parent), m_player(player), m_texture(nullptr), m_program(nullptr)
    {
        qDebug() << "VideoFrameCapture constructor";
        m_videoSink = new QVideoSink(this);
        m_player->setVideoSink(m_videoSink);

        connect(m_videoSink, &QVideoSink::videoFrameChanged, this, &VideoFrameCapture::onVideoFrameChanged);

        m_player->setAudioOutput(nullptr);

        connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
            qDebug() << "Media Status Changed: " << status;
            if (status == QMediaPlayer::LoadedMedia) {
                if (!m_player->isPlaying()) {
                    m_player->stop();
                    m_player->play();
                }
            }
        });

        // 延迟 OpenGL 初始化
        m_initialized = false;
    }

    ~VideoFrameCapture()
    {
        qDebug() << "Destroying VideoFrameCapture";
        if (m_texture) {
            delete m_texture;
        }
        delete m_program;
    }

protected:
    void initializeGL() override
    {
        qDebug() << "Initializing OpenGL";

        // Initialize OpenGL functions
        initializeOpenGLFunctions();

        // 延迟资源创建，避免初始化时出现问题
        if (!m_initialized) {
            m_program = new QOpenGLShaderProgram(this);
            if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
                "attribute vec4 vertex;\n"
                "attribute vec2 texCoord;\n"
                "varying vec2 fragTexCoord;\n"
                "uniform mat4 matrix;\n"
                "void main() {\n"
                "    gl_Position = matrix * vertex;\n"
                "    fragTexCoord = texCoord;\n"
                "}"))
            {
                qDebug() << "Failed to compile vertex shader: " << m_program->log();
                return;
            }

            if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                "uniform sampler2D texture;\n"
                "varying vec2 fragTexCoord;\n"
                "void main() {\n"
                "    gl_FragColor = texture2D(texture, fragTexCoord);\n"
                "}"))
            {
                qDebug() << "Failed to compile fragment shader: " << m_program->log();
                return;
            }

            if (!m_program->link()) {
                qDebug() << "Failed to link shader program: " << m_program->log();
                return;
            }

            m_program->bind();

            m_vertices << QVector2D(-1.0f, -1.0f) << QVector2D(1.0f, -1.0f) << QVector2D(1.0f, 1.0f) << QVector2D(-1.0f, 1.0f);
            m_texCoords << QVector2D(0.0f, 0.0f) << QVector2D(1.0f, 0.0f) << QVector2D(1.0f, 1.0f) << QVector2D(0.0f, 1.0f);

            m_vertexBuffer.create();
            m_vertexBuffer.bind();
            m_vertexBuffer.allocate(m_vertices.constData(), m_vertices.count() * sizeof(QVector2D));

            m_texCoordBuffer.create();
            m_texCoordBuffer.bind();
            m_texCoordBuffer.allocate(m_texCoords.constData(), m_texCoords.count() * sizeof(QVector2D));

            qDebug() << "OpenGL Initialized";
            m_initialized = true;
        }
    }

    void paintEvent(QPaintEvent *event) override
    {
        QWidget::paintEvent(event);

        if (!m_texture)
        {
            qDebug() << "Texture not initialized yet!";
            return;
        }

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QSize widgetSize = size();
        int offsetX = (widgetSize.width() - m_texture->width()) / 2;
        int offsetY = (widgetSize.height() - m_texture->height()) / 2;

        // 使用 OpenGL 渲染纹理
        painter.beginNativePainting();
        glClear(GL_COLOR_BUFFER_BIT);

        // 设置投影矩阵
        QMatrix4x4 matrix;
        matrix.ortho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f);
        matrix.translate(0.0f, 0.0f, -1.0f);

        // 绑定着色器程序
        m_program->bind();
        m_program->setUniformValue("matrix", matrix);
        m_program->setUniformValue("texture", 0);

        // 绑定纹理
        m_texture->bind();

        m_vertexBuffer.bind();
        m_program->enableAttributeArray(0);
        m_program->setAttributeBuffer(0, GL_FLOAT, 0, 2, 0);

        m_texCoordBuffer.bind();
        m_program->enableAttributeArray(1);
        m_program->setAttributeBuffer(1, GL_FLOAT, 0, 2, 0);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        painter.endNativePainting();
    }

    void onVideoFrameChanged(const QVideoFrame &frame)
    {
        qDebug() << "onVideoFrameChanged called";

        if (!frame.isValid()) {
            qDebug() << "Invalid frame";
            return;
        }

        QVideoFrame &nonConstFrame = const_cast<QVideoFrame &>(frame);

        nonConstFrame.map(QVideoFrame::ReadOnly);

        if (!canUpdateFrame(nonConstFrame)) {
            qDebug() << "Frame hasn't changed";
            nonConstFrame.unmap();
            return;
        }

        const uchar *yPlane = nonConstFrame.bits(0);
        const uchar *uPlane = nonConstFrame.bits(1);
        const uchar *vPlane = nonConstFrame.bits(2);

        int width = frame.width();
        int height = frame.height();

        // 释放旧纹理，避免内存泄漏
        if (m_texture) {
            delete m_texture;
        }

        // 创建纹理并上传数据
        m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        m_texture->setSize(width, height);
        m_texture->setFormat(QOpenGLTexture::R8_UNorm);
        m_texture->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, yPlane);

        nonConstFrame.unmap(); // 解锁帧

        update();
    }

    bool canUpdateFrame(const QVideoFrame &frame)
    {
        QByteArray currentFrameHash = generateFrameHash(frame);
        if (currentFrameHash == m_lastFrameHash) {
            return false;
        }

        m_lastFrameHash = currentFrameHash;
        return true;
    }

    QByteArray generateFrameHash(const QVideoFrame &frame)
    {
        QCryptographicHash hash(QCryptographicHash::Sha256);

        const uchar *yPlane = frame.bits(0);
        const uchar *uPlane = frame.bits(1);
        const uchar *vPlane = frame.bits(2);

        int yBytes = frame.bytesPerLine(0) * frame.height();
        int uvBytes = frame.bytesPerLine(1) * (frame.height() / 2);

        hash.addData(reinterpret_cast<const char*>(yPlane), yBytes);
        hash.addData(reinterpret_cast<const char*>(uPlane), uvBytes);
        hash.addData(reinterpret_cast<const char*>(vPlane), uvBytes);

        return hash.result();
    }

private:
    QMediaPlayer *m_player;
    QVideoSink *m_videoSink;
    QOpenGLTexture *m_texture;  // Texture for video
    QByteArray m_lastFrameHash;

    QOpenGLShaderProgram *m_program;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_texCoordBuffer;
    QVector<QVector2D> m_vertices;
    QVector<QVector2D> m_texCoords;
    bool m_initialized;
};
