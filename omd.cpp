#include "omd.h"
#include "ui_omd.h"

OMD::OMD(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::OMD)
{
    // set app style
    qApp->setStyle(QStyleFactory::create("Fusion"));
    // modify palette to dark
    currentPalette = qApp->palette();
    darkPalette.setColor(QPalette::Window,QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText,Qt::white);
    darkPalette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
    darkPalette.setColor(QPalette::Base,QColor(42,42,42));
    darkPalette.setColor(QPalette::AlternateBase,QColor(66,66,66));
    darkPalette.setColor(QPalette::ToolTipBase,Qt::white);
    darkPalette.setColor(QPalette::ToolTipText,Qt::white);
    darkPalette.setColor(QPalette::Text,Qt::white);
    darkPalette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
    darkPalette.setColor(QPalette::Dark,QColor(35,35,35));
    darkPalette.setColor(QPalette::Shadow,QColor(20,20,20));
    darkPalette.setColor(QPalette::Button,QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText,Qt::white);
    darkPalette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
    darkPalette.setColor(QPalette::BrightText,Qt::red);
    darkPalette.setColor(QPalette::Link,QColor(42,130,218));
    darkPalette.setColor(QPalette::Highlight,QColor(42,130,218));
    darkPalette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
    darkPalette.setColor(QPalette::HighlightedText,Qt::white);
    darkPalette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));
    qApp->setPalette(darkPalette);
    QFile qfDarkstyle(QString(":/darkstyle/darkstyle.qss"));
    if(qfDarkstyle.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // set stylesheet
        QString qsStylesheet = QString(qfDarkstyle.readAll());
        qApp->setStyleSheet(qsStylesheet);
        qfDarkstyle.close();
    }
    ui->setupUi(this);
    netManager = new QNetworkAccessManager(this);
    dataBuffer = new QByteArray();
    stream_player = new QMediaPlayer(this, QMediaPlayer::StreamPlayback);
    ui->stanice->setColumnCount(3);
    ui->country->setCurrentIndex(0);
    ui->volume->setValue(100);
    ui->v2->setText("100%");
    QStringList naslov;
    naslov<<"Logo"<<"Station name"<<"Stream URL";
    ui->stanice->setHorizontalHeaderLabels(naslov);
    QHeaderView *verticalHeader = ui->stanice->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(100);
    ui->stanice->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->stanice->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->stanice->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->stanice->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->country->setDisabled(true);
    on_find_clicked();
}

OMD::~OMD()
{
    delete stationReply;
    delete playerReply;
    delete streamReply;
    delete dataBuffer;
    delete netManager;
    delete ui;
}

void OMD::on_country_currentIndexChanged(int index)
{
    pages = 0;
    cpage = 0;
    ui->country->setDisabled(true);
    loading_txt = true;
    loading_img = true;
    while(ui->stanice->rowCount()>0){
        int red = ui->stanice->currentRow();
            ui->stanice->removeRow(red);
    }
    stationList.clear();
    imagesToDownload.clear();
    nameList.clear();
    getStationUrl();
}

void OMD::getStationUrl()
{
    QNetworkRequest request;
    switch (ui->country->currentIndex()) {
        case 0: name = "bih"; break;
        case 1: name = "crna-gora"; break;
        case 2: name = "hrvatska"; break;
        case 3: name = "makedonija"; break;
        case 4: name = "slovenija"; break;
        case 5: name = "srbija"; break;
    }
    request.setUrl(QUrl("https://www.radiostanica.com/"+name+"/radio-stanice"));
    stationReply = netManager->get(request);
    connect(stationReply,&QNetworkReply::finished,this,&OMD::parseForStation);
}

void OMD::parseForStation()
{
    if(stationReply->error()){
        qDebug() << "Err: " << stationReply->errorString(); //Todo: Handle error
    }
    else {
        dataBuffer->append(stationReply->readAll());
        regex.setPattern("(?:<figure>)([\\s\\S]*?)(?:<\\/figure>)");
        QRegularExpressionMatchIterator i = regex.globalMatch(QString(*dataBuffer));
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            url.setPattern("(?<=href=\\\")(.*?)(?=\\\")");
            QRegularExpressionMatch st_url = url.match(QString(match.captured(0)));
            if(st_url.hasMatch()){
                stationList.append("https://www.radiostanica.com"+st_url.captured(0));
                logo.setPattern("(?<=src=\")(.*?)(?=\\\")");
                QRegularExpressionMatch st_logo = logo.match(QString(match.captured(0)));
                if(st_logo.hasMatch())
                    imagesToDownload.append("https://www.radiostanica.com"+st_logo.captured(0));
                else
                    imagesToDownload.append("null");
                QTableWidgetItem *newItem2 = new QTableWidgetItem();
                QTableWidgetItem *newItem1 = new QTableWidgetItem();
                QTableWidgetItem *newItem0 = new QTableWidgetItem();
                cstation = ui->stanice->rowCount();
                ui->stanice->insertRow(cstation);
                ui->stanice->setItem(cstation,0,newItem0);
                ui->stanice->setItem(cstation,1,newItem1);
                ui->stanice->setItem(cstation,2,newItem2);
                //qDebug() <<"URL: " << st_url.captured(0);
            }
            //qDebug() << match.captured(0);
        }
        pages = 0;
        regex.setPattern("(?i)<a class=\\\"page\\\"([^>]+)>(.+?)<\\/a>");
        QRegularExpressionMatchIterator k = regex.globalMatch(QString(*dataBuffer));
        while(k.hasNext()){
            QRegularExpressionMatch ma = k.next();
            if(ma.hasMatch()){
                pages++;
            }
        }
        dataBuffer->clear();
        if(cpage < pages){
            cpage++;
            QNetworkRequest request;
            request.setUrl(QUrl("https://www.radiostanica.com/"+name+"/radio-stanice?strana="+QString::number(cpage)));
            stationReply = netManager->get(request);
            connect(stationReply,&QNetworkReply::finished,this,&OMD::parseForStation);
        }
        else
            populatePlaylist();
    }
}

void OMD::populatePlaylist()
{
    cstation = 0;
    getPlayerUrl(stationList.at(0));
    currentImage = 0;
    getImage(imagesToDownload.at(0));
    ui->stanice->setCurrentCell(0,0);
}

void OMD::getPlayerUrl(QString station)
{
    QNetworkRequest request;
    request.setUrl(QUrl(station));
    playerReply = netManager->get(request);
    connect(playerReply,&QNetworkReply::finished,this,&OMD::parseForPlayer);
}

void OMD::parseForPlayer()
{
    if(playerReply->error()){
        qDebug() << "Err: " << playerReply->errorString(); //Todo: Handle error
    }
    else {
        dataBuffer->append(playerReply->readAll());
        regex.setPattern("(?<=href=\\\")(.*?)(?=\\\" rel=)");
        QRegularExpressionMatch match = regex.match(QString(*dataBuffer));
        if(match.hasMatch())
            getStreamUrl(match.captured(0));
    }
    dataBuffer->clear();
}

void OMD::getStreamUrl(QString player)
{
    QNetworkRequest request;
    request.setUrl(QUrl(player));
    streamReply = netManager->get(request);
    connect(streamReply,&QNetworkReply::finished,this,&OMD::parseForStream);
}

void OMD::parseForStream()
{
    if(streamReply->error()){
        qDebug() << "Err: " << streamReply->errorString(); //Todo: Handle error
    }
    else {
        dataBuffer->append(streamReply->readAll());
        regex.setPattern("(?<=h2>)(.*?)(?=</h2>)");
        QRegularExpressionMatch sname = regex.match(QString(*dataBuffer));
        if(sname.hasMatch())
            ui->stanice->item(cstation,1)->setText(sname.captured(0));
        else
            nameList.append(" ");
        regex.setPattern("(?<=file: ')(.*?)(?=;\\*)|(?<=file: ')(.*?)(?=')|(?<=file: ')(.*?)(?=;',)");
        QRegularExpressionMatch match = regex.match(QString(*dataBuffer));
        if(match.hasMatch())
            ui->stanice->item(cstation,2)->setText(match.captured(0));
        else
            streamList.append(" ");
    }
    dataBuffer->clear();
    if(cstation < ui->stanice->rowCount()-1){
        loading_txt = true;
        cstation++;
        getPlayerUrl(stationList.at(cstation));
    }
    else
        loading_txt = false;
    if(!loading_txt || !loading_img)
        ui->country->setDisabled(false);
}

void OMD::getImage(QString url)
{
    QNetworkRequest request;
    QString logo = url;
    request.setUrl(QUrl(logo));
    imageReply = netManager->get(request);
    connect(imageReply,&QNetworkReply::finished,this,&OMD::loadImages);
    //qDebug() <<"LOGO: " << logo;
}

void OMD::loadImages()
{
    if(imageReply->error()){
        qDebug() << "Err: " << imageReply->errorString(); //Todo: Handle error
    }
    else{
        QPixmap* img2 = new QPixmap();
        img2->loadFromData(imageReply->readAll());
        if(!img2->isNull())
            ui->stanice->item(currentImage,0)->setData(Qt::DecorationRole, img2->scaled(100,100, Qt::KeepAspectRatio));
    }
    if(currentImage < ui->stanice->rowCount()-1){
        loading_img = true;
        currentImage++;
        getImage(imagesToDownload.at(currentImage));
    }
    else
        loading_img = false;
}

void OMD::playFile()
{
    if(playing){
        stream_player->stop();
        playing = false;
    }
    stream_player->setMedia(QUrl(streamUrl));
    stream_player->play();
    playing = true;
}

void OMD::on_find_clicked()
{
    while(ui->stanice->rowCount()>0){
        int red = ui->stanice->currentRow();
            ui->stanice->removeRow(red);
    }
    stationList.clear();
    imagesToDownload.clear();
    getStationUrl();
}

void OMD::stopPlay()
{
    if(playing){
        stream_player->stop();
        playing = false;
    }
}

void OMD::on_volume_sliderMoved(int position)
{
    ui->v2->setText(QString::number(position)+"%");
    stream_player->setVolume(position);
}

void OMD::on_stanice_itemDoubleClicked(QTableWidgetItem *item)
{
    int row = item->row();
    streamUrl = ui->stanice->item(row,2)->text();
    ui->stream->setText(streamUrl);
    ui->name->setText(ui->stanice->item(row,1)->text());
    ui->logo->setPixmap(ui->stanice->item(row,0)->data(Qt::DecorationRole).value<QPixmap>());
    playFile();
}

void OMD::on_stop_clicked()
{
    stopPlay();
}

void OMD::on_play_clicked()
{
    if(!streamUrl.isEmpty())
        playFile();
}

void OMD::about()
{
    QMessageBox about_box(this);
    QString bno = QString("%1%2")
            .arg(QLocale(QLocale::C).toDate(QString(__DATE__).simplified(), QLatin1String("MMM d yyyy")).toString("yyyyMMdd"))
            .arg(QString("%1%2%3%4").arg(__TIME__[0])
            .arg(__TIME__[1])
            .arg(__TIME__[3])
            .arg(__TIME__[4]));
    about_box.setWindowTitle("About OMD");
    QString text = QString("<html><head/><body><p><span style=' font-weight:600;'><big>Internet radio player</big><br/>Version ");
    text.append(VERSION);
    text.append("</span><br/><span style=' font-weight:600;'>Build: </span>"+bno+"<br/><span style=' font-weight:600;'>Copyright © 2021 Prša Igor</span> (<a href=&quot;mailto:akiro.ip@gmail.com&quot;>akiro.ip@gmail.com</a>)</p><p>This program is distributed in the hope that it will be useful,<br/>but WITHOUT ANY WARRANTY; without even the implied<br/>warranty of MERCHANTABILITY or FITNESS FOR<br/>A PARTICULAR PURPOSE.</p></body></html>");
    about_box.setText(text);
    about_box.setIconPixmap(QPixmap(":/streaming.png").scaled(64,64));
    about_box.setParent(this);
    about_box.exec();
}

void OMD::aboutQt()
{
    QApplication::aboutQt();
}

void OMD::on_OMD_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->centralwidget->mapToGlobal(pos);
    QMenu localMenu;
    localMenu.addAction("About", this, SLOT(about()));
    localMenu.addAction("About Qt", this, SLOT(aboutQt()));
    localMenu.exec(globalPos);
}
