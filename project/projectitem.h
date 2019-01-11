#ifndef PROJECTITEM_H
#define PROJECTITEM_H

#include <QString>
#include <memory>

class Sequence;
class Footage;

namespace project {

    class ProjectItem
    {
    public:
        ProjectItem();
        virtual ~ProjectItem();
        virtual void setName(const QString val);
        virtual const QString& getName() const;

    private:
        friend class ::Sequence;
        friend class ::Footage;
        QString name;
    };

    typedef std::shared_ptr<ProjectItem> ProjectItemPtr;
}
#endif // PROJECTITEM_H
