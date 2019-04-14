#include "footagestream.h"

#include <QPainter>

#include "debug.h"

using project::FootageStream;

void FootageStream::make_square_thumb()
{
  // generate square version for QListView?
  const auto max_dimension = qMax(video_preview.width(), video_preview.height());
  QPixmap pixmap(max_dimension, max_dimension);
  pixmap.fill(Qt::transparent);
  QPainter p(&pixmap);
  const auto diff = (video_preview.width() - video_preview.height()) / 2;
  const auto sqx = (diff < 0) ? -diff : 0;
  const auto sqy = (diff > 0) ? diff : 0;
  p.drawImage(sqx, sqy, video_preview);
  video_preview_square = QIcon(pixmap);
}

void FootageStream::load(const QXmlStreamReader& stream)
{
  // TODO:
}

bool FootageStream::save(QXmlStreamWriter& stream) const
{
  switch (type_) {
    case StreamType::VIDEO:
      stream.writeStartElement("video");
      stream.writeAttribute("id", QString::number(file_index));
      stream.writeAttribute("width", QString::number(video_width));
      stream.writeAttribute("height", QString::number(video_height));
      stream.writeAttribute("framerate", QString::number(video_frame_rate, 'f', 10));
      stream.writeAttribute("infinite", QString::number(infinite_length));
      stream.writeEndElement();
      return true;
    case StreamType::AUDIO:
      stream.writeStartElement("audio");
      stream.writeAttribute("id", QString::number(file_index));
      stream.writeAttribute("channels", QString::number(audio_channels));
      stream.writeAttribute("layout", QString::number(audio_layout));
      stream.writeAttribute("frequency", QString::number(audio_frequency));
      stream.writeEndElement();
      return true;
    default:
      qWarning() << "Unknown stream type. Not saving";
      break;
  }
  return false;
}
