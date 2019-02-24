#include "mediahandler.h"

using project::MediaHandler;

MediaHandler::~MediaHandler()
{
  // FIXME: manage data
  format_ctx_ = nullptr;
  stream_ = nullptr;
  codec_ = nullptr;
  codec_ctx_ = nullptr;
  packet_ = nullptr;
  frame_ = nullptr;
  opts_ = nullptr;
}
