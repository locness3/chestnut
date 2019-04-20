#ifndef VOIDEFFECT_H
#define VOIDEFFECT_H

/* VoidEffect is a placeholder used when Chesnut is unable to find an effect
 * requested by a loaded project. It displays a missing effect so the user knows
 * an effect is missing, and stores the XML project data verbatim so that it
 * isn't lost if the user saves over the project.
 */

#include "project/effect.h"

class VoidEffect : public Effect {
public:
    VoidEffect(ClipPtr c, const QString& n);
    bool load(QXmlStreamReader &stream) override;
    bool save(QXmlStreamWriter &stream) const override;

    virtual void setupUi() override;
private:
    QByteArray bytes;
};

#endif // VOIDEFFECT_H
