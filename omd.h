#ifndef OMD_H
#define OMD_H

#include <QMainWindow>
#include <QtNetwork/QNetworkAccessManager>
#include <QRegularExpression>
#include <QtNetwork/QNetworkReply>
#include <QDebug>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantMap>
#include <QProcess>
#include <QtMultimedia/QMediaPlayer>
#include <QMenu>
#include <QFile>
#include <QTableWidgetItem>
#include <QStyleFactory>

#define VERSION "0.5.0"

QT_BEGIN_NAMESPACE
namespace Ui { class OMD; }
QT_END_NAMESPACE

class OMD : public QMainWindow
{
    Q_OBJECT

public:
    OMD(QWidget *parent = nullptr);
    ~OMD();

private slots:
    void getImage(QString url);
    void getStationUrl();
    void getStreamUrl(QString player);
    void getPlayerUrl(QString station);
    void loadImages();
    void parseForStream();
    void parseForPlayer();
    void parseForStation();
    void populatePlaylist();
    void playFile();
    void stopPlay();
    void on_find_clicked();
    void on_volume_sliderMoved(int position);
    void on_country_currentIndexChanged(int index);
    void on_stanice_itemDoubleClicked(QTableWidgetItem *item);
    void about();
    void aboutQt();
    void on_stop_clicked();
    void on_play_clicked();
    void on_OMD_customContextMenuRequested(const QPoint &pos);

private:
    Ui::OMD *ui;
    QPalette darkPalette, currentPalette;
    QNetworkAccessManager *netManager, *imageManager;
    QNetworkReply *stationReply = nullptr, *playerReply = nullptr, *streamReply = nullptr, *imageReply = nullptr;
    QByteArray *dataBuffer;
    QString name;
    QRegularExpression regex, logo, url;
    QString streamUrl;
    QMediaPlayer *stream_player;
    QStringList imagesToDownload, stationList, nameList, streamList;
    int currentImage, cstation, pages, cpage = 0;
    bool playing = false, loading_img = false, loading_txt = false;
};
#endif // OMD_H
