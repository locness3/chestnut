#ifndef SEQUENCEITEM_H
#define SEQUENCEITEM_H

#include <memory>
#include <QString>

namespace project {
    enum SequenceItemType_E {
        SEQUENCE_ITEM_CLIP = 0,
        SEQUENCE_ITEM_EFFECT = 1,
        SEQUENCE_ITEM_NONE
    };

    class SequenceItem
    {
    public:
        SequenceItem();
        virtual ~SequenceItem();

        void setName(const QString& val);
        const QString& getName() const;

        virtual SequenceItemType_E getType() const;

    private:
        friend class Clip;
        friend class Effect;
        QString name;

    };
    typedef std::shared_ptr<SequenceItem> SequenceItemPtr;
}

#endif // SEQUENCEITEM_H
