#include "sequenceitem.h"

using project::SequenceItem;

SequenceItem::SequenceItem()
{

}

SequenceItem::~SequenceItem()
{

}


void SequenceItem::setName(const QString& val) {
    name = val;
}
const QString& SequenceItem::getName() const {
    return name;
}


project::SequenceItemType_E SequenceItem::getType() const {
    return project::SEQUENCE_ITEM_NONE;
}
