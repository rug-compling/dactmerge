#include <QDirIterator>
#include <QFileDialog>
#include <QFuture>
#include <QList>
#include <QListWidgetItem>
#include <QPair>
#include <QProgressDialog>
#include <QScopedPointer>
#include <QString>
#include <QtConcurrentRun>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusWriter.hh>

#include <MainWindow.hh>

#include <ui_MainWindow.h>

namespace ac = alpinocorpus;

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
d_ui(new Ui::MainWindow),
d_saveProgressDialog(new QProgressDialog(this))
{
    setupUi();
    setupConnections();
}

MainWindow::~MainWindow()
{
    delete d_saveProgressDialog;
    delete d_ui;
}

void MainWindow::addDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    "Add corpora from directory");
    if (dir.isNull())
        return;
    
    QDirIterator iter(dir, QDirIterator::Subdirectories |
                      QDirIterator::FollowSymlinks);
    while (iter.hasNext()) {
        QString entry = iter.next();
        
        if (entry.endsWith(".index") || entry.endsWith(".dact")) {
            QListWidgetItem *item = new QListWidgetItem(entry);
            item->setData(Qt::UserRole, dir);
            d_ui->corpusListWidget->addItem(item);
        }
    }
}

void MainWindow::cancelSaveCorpus()
{
    d_saveCorpusCancelled = true;
}

void MainWindow::corpusSaved()
{
    d_saveProgressDialog->accept();
}

void MainWindow::setupConnections()
{
    connect(d_ui->addDirectoryAction, SIGNAL(activated()),
            SLOT(addDirectory()));
    connect(d_ui->saveAction, SIGNAL(activated()),
            SLOT(saveToCorpus()));
    connect(this, SIGNAL(saveProgress(int)), d_saveProgressDialog, SLOT(setValue(int)));
    connect(this, SIGNAL(saveProgressMaximum(int)), d_saveProgressDialog, SLOT(setMaximum(int)));
    connect(d_saveProgressDialog, SIGNAL(canceled()), SLOT(cancelSaveCorpus()));
    connect(&d_saveWatcher, SIGNAL(resultReadyAt(int)), SLOT(corpusSaved()));
}

void MainWindow::saveToCorpus()
{
    if (d_saveWatcher.isRunning()) {
        d_saveWatcher.cancel();
        d_saveWatcher.waitForFinished();
    }
    
    QString filename(QFileDialog::getSaveFileName(this,
                                                  "Save to corpus", QString(), "*.dact"));
    if (filename.isNull())
        return;
    
    d_saveCorpusCancelled = false;
    d_saveProgressDialog->setWindowTitle("Saving to corpus");
    d_saveProgressDialog->setLabelText(QString("Saving to %1").arg(filename));
    d_saveProgressDialog->open();
    
    QList<QPair<QString, QString> > corpora;

    for (int i = 0; i < d_ui->corpusListWidget->count(); ++i) {
        QListWidgetItem *item = d_ui->corpusListWidget->item(i);
        
        QString corpus(item->text());
        QString dir(item->data(Qt::UserRole).toString());

        corpora.push_back(QPair<QString, QString>(corpus, dir));
    }
    
    QFuture<void> future = QtConcurrent::run(this, &MainWindow::doSaveToCorpus, filename, corpora);
    d_saveWatcher.setFuture(future);
}

void MainWindow::doSaveToCorpus(QString filename, QList<QPair<QString, QString> > corpora)
{
    emit saveProgressMaximum(corpora.size());
    emit saveProgress(0);
        
    ac::DbCorpusWriter writer(filename.toUtf8().constData(), true);
    
    for (int i = 0; i < corpora.size(); ++i) {
        QString corpus(corpora.at(i).first);
        QString dir(corpora.at(i).second);
        
        QString basePath(corpus);
        basePath.remove(0, dir.size());
        if (basePath[0] == '/')
            basePath.remove(0, 1);
        
        if (basePath.endsWith(".index"))
            basePath.remove(basePath.size() - 6, 6);
        else if (basePath.endsWith(".dact"))
            basePath.remove(basePath.size() - 5, 5);
        
        QScopedPointer<ac::CorpusReader> reader(
                                                ac::CorpusReader::open(corpus.toUtf8().constData()));
        for (ac::CorpusReader::EntryIterator entryIter = reader->begin();
             entryIter != reader->end(); ++entryIter)
        {
            QString entry = QString("%1/%2").arg(basePath).arg(QString::fromUtf8((*entryIter).c_str()));
            writer.write(entry.toUtf8().constData(), reader->read(*entryIter));
        }
        
        if (d_saveCorpusCancelled)
            break;
        
        emit saveProgress(i + 1);
    }    
}

void MainWindow::setupUi()
{
    d_ui->setupUi(this);
}
