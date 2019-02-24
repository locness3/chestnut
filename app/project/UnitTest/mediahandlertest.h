#ifndef MEDIAHANDLERTEST_H
#define MEDIAHANDLERTEST_H

#include <QObject>

class MediaHandlerTest : public QObject
{
    Q_OBJECT
  public:
    explicit MediaHandlerTest(QObject *parent = nullptr);

  signals:

  private slots:
    void testCaseConstructor();

};

#endif // MEDIAHANDLERTEST_H
