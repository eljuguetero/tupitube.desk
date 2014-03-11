#include "tupcamerawindow.h"
#include "tdebug.h"

#include <QVideoEncoderControl>

struct TupCameraWindow::Private
{
    QCamera *camera;
    QCameraImageCapture *imageCapture;
    TupVideoSurface *videoSurface;
    QString dir;
    int counter;
};

TupCameraWindow::TupCameraWindow(QCamera *input, const QSize &camResolution, const QSize &displayResolution, QCameraImageCapture *imageCapture, 
                                 const QString &path, QWidget *parent) : QWidget(parent), k(new Private)
{
    setFixedSize(displayResolution + QSize(1, 1));

    k->dir = path;

    k->camera = input;
    k->imageCapture = imageCapture;
    k->camera->setCaptureMode(QCamera::CaptureStillImage);

    connect(k->camera, SIGNAL(error(QCamera::Error)), this, SLOT(error(QCamera::Error)));
    connect(k->imageCapture, SIGNAL(imageSaved(int, const QString)), this, SLOT(imageSavedFromCamera(int, const QString)));

    QMediaService *service = k->camera->service();

    QVideoEncoderControl *encoderControl = service->requestControl<QVideoEncoderControl*>();
    QVideoEncoderSettings settings = encoderControl->videoSettings();
    settings.setResolution(camResolution);
    encoderControl->setVideoSettings(settings);

    QVideoRendererControl *rendererControl = service->requestControl<QVideoRendererControl*>();

    bool isScaled = false;
    if (camResolution != displayResolution)
        isScaled = true;

    k->videoSurface = new TupVideoSurface(this, this, displayResolution, isScaled, this);
    rendererControl->setSurface(k->videoSurface);
}

TupCameraWindow::~TupCameraWindow()
{
}

void TupCameraWindow::startCamera()
{
    stopCamera();
    k->camera->start();
}

void TupCameraWindow::stopCamera()
{
    if (k->camera->state() == QCamera::ActiveState)
        k->camera->stop();
}

void TupCameraWindow::reset()
{
    QDir dir(k->dir);
    foreach (QString file, dir.entryList(QStringList() << "*.jpg")) {
             QString absolute = dir.absolutePath() + QDir::separator() + file;
             QFile::remove(absolute);
    }

    if (! dir.rmdir(dir.absolutePath())) {
        #ifdef K_DEBUG
               tError() << "TupCameraInterface::closeEvent() - Fatal Error: Can't remove pictures directory -> " << dir.absolutePath();
        #endif
    }

    if (k->videoSurface)
        k->videoSurface->stop();

    if (k->camera->state() == QCamera::ActiveState)
        k->camera->stop();
}

void TupCameraWindow::error(QCamera::Error error)
{
    switch (error) {
            case QCamera::NoError:
            {
                break;
            }
            case QCamera::CameraError:
            {
                QMessageBox::warning(this, "TupCameraWindow", "General Camera error");
                break;
            }
            case QCamera::InvalidRequestError:
            {
                QMessageBox::warning(this, "TupCameraWindow", "Camera invalid request error");
                break;
            }
            case QCamera::ServiceMissingError:
            {
                QMessageBox::warning(this, "TupCameraWindow", "Camera service missing error");
                break;
            }
            case QCamera::NotSupportedFeatureError :
            {
                QMessageBox::warning(this, "TupCameraWindow", "Camera not supported error");
                break;
            }
    };
}

void TupCameraWindow::updateVideo()
{
    repaint();
}

void TupCameraWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    if (k->videoSurface && k->videoSurface->isActive())
        k->videoSurface->paint(&painter);
}

void TupCameraWindow::takePicture(int counter)
{
    QString prev = "pic";
    if (counter < 10)
        prev += "00";
    if (counter > 10 && counter < 100)
        prev += "0";

    QString imagePath = k->dir + QDir::separator() + prev + QString::number(counter) + ".jpg";

    tError() << "TupCameraWindow::takePicture() - imagePath: " << imagePath;

    //on half pressed shutter button
    k->camera->searchAndLock();

    //on shutter button pressed
    k->imageCapture->capture(imagePath);

    //on shutter button released
    k->camera->unlock();

    k->counter = counter;
}

void TupCameraWindow::imageSavedFromCamera(int id, const QString path)
{
    Q_UNUSED(id);

    tError() << "TupCameraInterface::imageSavedFromCamera() - ID: " << k->counter;
    tError() << "TupCameraInterface::imageSavedFromCamera() - Image saved from Camera at: " << path;
    if (path.isEmpty())
        return;

    emit pictureHasBeenSelected(k->counter, path);
    k->videoSurface->setLastImage(QImage(path));
}

void TupCameraWindow::drawGrid(bool flag)
{
    k->videoSurface->drawGrid(flag);
}

void TupCameraWindow::drawActionSafeArea(bool flag)
{
    k->videoSurface->drawActionSafeArea(flag);
}

void TupCameraWindow::showHistory(bool flag)
{
    k->videoSurface->showHistory(flag);
}

