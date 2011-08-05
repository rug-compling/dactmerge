#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSharedPointer>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();
private slots:
  void addDirectory();
  void saveToCorpus();
private:
  void setupUi();
  void setupConnections();

  Ui::MainWindow *d_ui;
};

#endif // MAINWINDOW_H
