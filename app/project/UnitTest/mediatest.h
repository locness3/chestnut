#ifndef MEDIATEST_H
#define MEDIATEST_H

#include <QtTest>
#include <QObject>

namespace chestnut::project
{

class MediaTest : public QObject
{
    Q_OBJECT
public:
    MediaTest();

signals:

private slots:    
    /**
     * @brief Construct default media instance and check class attributes are correct
     */
    void testCaseConstructor();
    /**
     * @brief Set media instance as a child of another instance and check linkage is correct
     */
    void testCaseConstructorWithParent();
    /**
     * @brief Set media instance as a Footage type and check stored object is non-null
     */
    void testCaseSetAsFootage();
    /**
     * @brief Set media instance as a Sequence type and check stored object is non-null
     */
    void testCaseSetAsSequence();
    /**
     * @brief Set media instance as a folder and check name is non-empty
     */
    void testCaseSetAsFolder();
    /**
     * @brief Clearing of media sets it to none-type and the stored object is empty/null/invalid
     */
    void testCaseClear();
    /**
     * @brief Check that when its id is set Media::nextID is greater
     */
    void testCaseSetId();
};
}
#endif // MEDIATEST_H
