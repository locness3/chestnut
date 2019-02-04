#include "sequenceitem.h"

using project::SequenceItem;

SequenceItem::SequenceItem()
    : name(),
      type(project::SequenceItemType::NONE)
{

}

SequenceItem::~SequenceItem()
{

}


SequenceItem::SequenceItem(const project::SequenceItemType sequenceType)
    : name(),
      type(sequenceType)
{

}


void SequenceItem::setName(const QString& val) {
    //TODO: input validation
    name = val;
}
const QString& SequenceItem::getName() const {
    return name;
}


project::SequenceItemType SequenceItem::getType() const {
    return type;
}
