#ifndef SEQUENCEITEM_H
#define SEQUENCEITEM_H

#include <memory>
#include <QString>

class Clip;
class Effect;

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
        explicit SequenceItem(const SequenceItemType sequenceType);
        virtual ~SequenceItem() = default;
        SequenceItem(const SequenceItem&) = delete;
        SequenceItem(const SequenceItem&&) = delete;
        SequenceItem& operator=(const SequenceItem&) = delete;
        SequenceItem& operator=(const SequenceItem&&) = delete;

        void setName(const QString& val);
        const QString& name() const;

        SequenceItemType type() const;

    private:
        friend class ::Clip;
        friend class ::Effect;
        QString name_;
        SequenceItemType type_;

    };
    typedef std::shared_ptr<SequenceItem> SequenceItemPtr;
}

#endif // SEQUENCEITEM_H
