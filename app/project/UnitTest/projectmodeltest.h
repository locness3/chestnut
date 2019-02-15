#ifndef PROJECTMODELTEST_H
#define PROJECTMODELTEST_H

#include <QObject>

class ProjectModelTest : public QObject
{
    Q_OBJECT
  public:
    explicit ProjectModelTest(QObject *parent = nullptr);

  private slots:
    void testCaseInsert();
    void testCaseAppendChildWithSequence();
};

#endif // PROJECTMODELTEST_H
