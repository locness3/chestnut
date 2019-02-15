#include "projectmodeltest.h"
#include "project/projectmodel.h"

#include <QtTest>

ProjectModelTest::ProjectModelTest(QObject *parent) : QObject(parent)
{

}

void ProjectModelTest::testCaseInsert()
{
  ProjectModel model;
  auto item = std::make_shared<Media>();
  QVERIFY(model.project_items.empty());
  model.insert(item);
  QVERIFY(model.project_items.size() == 1);
}

void ProjectModelTest::testCaseAppendChildWithSequence()
{
  ProjectModel model;
  QVERIFY(model.project_items.empty());
  auto parnt = std::make_shared<Media>();
  auto chld = std::make_shared<Media>();
  auto sqn = std::make_shared<Sequence>();
  chld->setSequence(sqn);
  model.appendChild(parnt, chld);
  QVERIFY(model.project_items.size() == 1);
}
