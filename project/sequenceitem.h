#ifndef SEQUENCEITEM_H
#define SEQUENCEITEM_H

#include <memory>
#include <QString>

namespace project {
    enum class SequenceItemType {
        CLIP = 0,
        EFFECT = 1,
        NONE
    };

    class SequenceItem
    {
    public:
        SequenceItem();
        virtual ~SequenceItem();
        SequenceItem(const SequenceItem&) = delete;
        SequenceItem(const SequenceItem&&) = delete;
        SequenceItem& operator=(const SequenceItem&) = delete;
        SequenceItem& operator=(const SequenceItem&&) = delete;

        void setName(const QString& val);
        const QString& getName() const;

        virtual SequenceItemType getType() const;

    private:
        friend class Clip;
        friend class Effect;
        QString name;

    };
    typedef std::shared_ptr<SequenceItem> SequenceItemPtr;
}

#endif // SEQUENCEITEM_H
