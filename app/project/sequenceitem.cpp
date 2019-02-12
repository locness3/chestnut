#include "sequenceitem.h"

using project::SequenceItem;

SequenceItem::SequenceItem()
  : name_(),
    type_(project::SequenceItemType::NONE)
{

}

SequenceItem::~SequenceItem()
{

}


SequenceItem::SequenceItem(const project::SequenceItemType sequenceType)
  : name_(),
    type_(sequenceType)
{

}


void SequenceItem::setName(const QString& val)
{
  //TODO: input validation
  name_ = val;
}
const QString& SequenceItem::name() const
{
  return name_;
}


project::SequenceItemType SequenceItem::type() const
{
  return type_;
}
