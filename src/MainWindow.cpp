#include <QDirIterator>
#include <QFileDialog>
#include <QList>
#include <QListWidgetItem>
#include <QScopedPointer>
#include <QString>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusWriter.hh>

#include <MainWindow.hh>

#include <ui_MainWindow.h>

namespace ac = alpinocorpus;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  d_ui(new Ui::MainWindow)
{
  setupUi();
  setupConnections();
}

MainWindow::~MainWindow()
{
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

void MainWindow::setupConnections()
{
  connect(d_ui->addDirectoryAction, SIGNAL(activated()),
      SLOT(addDirectory()));
  connect(d_ui->saveAction, SIGNAL(activated()),
      SLOT(saveToCorpus()));
}

void MainWindow::saveToCorpus()
{
  QString filename(QFileDialog::getSaveFileName(this,
        "Save to corpus", QString(), "*.dact"));
  if (filename.isNull())
    return;

  ac::DbCorpusWriter writer(filename.toUtf8().constData(), true);

  for (int i = 0; i < d_ui->corpusListWidget->count(); ++i) {
    QListWidgetItem *item = d_ui->corpusListWidget->item(i);

    QString corpus(item->text());
    QString dir(item->data(Qt::UserRole).toString());

    QString basePath(corpus);
    basePath.remove(0, dir.size());
    if (basePath[0] == '/')
      basePath.remove(0, 1);
    // XXX - extension
    
    QScopedPointer<ac::CorpusReader> reader(
      ac::CorpusReader::open(corpus.toUtf8().constData()));
    for (ac::CorpusReader::EntryIterator entryIter = reader->begin();
        entryIter != reader->end(); ++entryIter)
    {
      QString entry = QString("%1/%2").arg(basePath).arg(QString::fromUtf8((*entryIter).c_str()));
      writer.write(entry.toUtf8().constData(), reader->read(*entryIter));
    }
  }
}

void MainWindow::setupUi()
{
  d_ui->setupUi(this);
}
