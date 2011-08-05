#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFutureWatcher>
#include <QList>
#include <QMainWindow>
#include <QPair>
#include <QSharedPointer>
#include <QString>

namespace Ui {
    class MainWindow;
}

class QProgressDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
signals:
    void saveProgressMaximum(int max);
    void saveProgress(int progress);
    
private slots:
    void addDirectory();
    void cancelSaveCorpus();
    void doSaveToCorpus(QString filename, QList<QPair<QString, QString> > corpora);
    
    void saveToCorpus();
private:
    void setupUi();
    void setupConnections();
    
    Ui::MainWindow *d_ui;
    QProgressDialog *d_saveProgressDialog;
    QFutureWatcher<void> d_saveWatcher;
    bool d_saveCorpusCancelled;
};

#endif // MAINWINDOW_H
