#include "sequenceitem.h"

using project::SequenceItem;

SequenceItem::SequenceItem()
    : _name(),
      _type(project::SequenceItemType::NONE)
{

}

SequenceItem::~SequenceItem()
{

}


SequenceItem::SequenceItem(const project::SequenceItemType sequenceType)
    : _name(),
      _type(sequenceType)
{

}


void SequenceItem::setName(const QString& val) {
    //TODO: input validation
    _name = val;
}
const QString& SequenceItem::getName() const {
    return _name;
}


project::SequenceItemType SequenceItem::getType() const {
    return _type;
}
