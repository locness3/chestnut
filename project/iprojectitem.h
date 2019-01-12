#ifndef IPROJECTITEM_H
#define IPROJECTITEM_H

#include <QString>

class IProjectItem
{
public:
    virtual ~IProjectItem() {}

    virtual void setName(const QString val) = 0;
    virtual const QString& getName() const = 0;

}
#endif // IPROJECTITEM_H
