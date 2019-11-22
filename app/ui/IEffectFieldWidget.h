#ifndef IEFFECTFIELDWIDGET_H
#define IEFFECTFIELDWIDGET_H

#include <QVariant>

namespace chestnut::ui
{
  class IEffectFieldWidget
  {
    public:
      IEffectFieldWidget() {}
      virtual ~IEffectFieldWidget() {}

      virtual QVariant getValue() const = 0;

      virtual void setValue(QVariant val) = 0;
  };
}

#endif // IEFFECTFIELDWIDGET_H
