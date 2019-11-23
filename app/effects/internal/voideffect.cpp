#include "voideffect.h"

#include <QXmlStreamReader>
#include <QLabel>
#include <QFile>

#include "ui/collapsiblewidget.h"
#include "debug.h"

VoidEffect::VoidEffect(ClipPtr c, const QString& n)
  : Effect(c)
{
  setName(n);
}

bool VoidEffect::load(QXmlStreamReader &stream)
{
  QString tag = stream.name().toString();
  qint64 start_index = stream.characterOffset();
  qint64 end_index = start_index;
  while (!stream.atEnd() && !(stream.name() == tag && stream.isEndElement())) {
    end_index = stream.characterOffset();
    stream.readNext();
  }
  qint64 passage_length = end_index - start_index;
  if (passage_length > 0) {
    // store xml data verbatim
    auto device = dynamic_cast<QFile*>(stream.device());

    QFile passage_get(device->fileName());
    if (passage_get.open(QFile::ReadOnly)) {
      passage_get.seek(start_index);
      bytes = passage_get.read(passage_length);
      int passage_end = bytes.lastIndexOf('>')+1;
      bytes.remove(passage_end, bytes.size()-passage_end);
      passage_get.close();
    }
  }
  return false;
}

bool VoidEffect::save(QXmlStreamWriter &stream) const
{
  // TODO:
//  if (!name_.isEmpty()) {
//    stream.writeAttribute("name", name_);
//    stream.writeAttribute("enabled", QString::number(is_enabled()));

//    // force xml writer to expand <effect> tag, ignored when loading
//    stream.writeStartElement("void");
//    stream.writeEndElement();

//    if (!bytes.isEmpty()) {
//      // write stored data
//      QIODevice* device = stream.device();
//      device->write(bytes);
//    }
//  }
  return false;
}

void VoidEffect::setupUi()
{
  if (ui_setup) {
    return;
  }
  Effect::setupUi();

  QString display_name;
  if (name().isEmpty()) {
    display_name = tr("(unknown)");
  } else {
    display_name = name();
  }
  EffectRowPtr row = add_row(tr("Missing Effect"), false, false);
  row->add_widget(new QLabel(display_name));
  container->setText(display_name);

}
