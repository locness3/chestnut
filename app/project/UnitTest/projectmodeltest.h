#ifndef PROJECTMODELTEST_H
#define PROJECTMODELTEST_H

#include <QObject>

class ProjectModelTest : public QObject
{
    Q_OBJECT
  public:
    explicit ProjectModelTest(QObject *parent = nullptr);

  private slots:
    void testCaseConstructor();
    void testCaseInsert();
    void testCaseAppendChildWithSequence();
    /**
     * Test the get of a folder from an unpopulated model
     */
    void testCaseGetFolder();
    /**
     * Test the get of a folder from an populated model
     */
    void testCaseGetFolderPopulated();
};

#endif // PROJECTMODELTEST_H
