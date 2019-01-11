#include "projectitem.h"

using project::ProjectItem;

ProjectItem::ProjectItem()
{

}

ProjectItem::~ProjectItem()
{

}

void ProjectItem::setName(const QString val)
{
    name = val;
}
const QString& ProjectItem::getName() const
{
    return name;
}
