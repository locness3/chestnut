#include "footagetest.h"
#include "project/footage.h"

FootageTest::FootageTest(QObject *parent) : QObject(parent)
{

}


void FootageTest::testCaseIsImage()
{
  Footage ftg;
  // by default, not an image
  QVERIFY(ftg.isImage() == false);
  // the sole stream is a video-type and is infinitely long
  auto stream = std::make_shared<FootageStream>();
  stream->infinite_length = true;
  ftg.video_tracks.append(stream);
  QVERIFY(ftg.isImage() == true);
}
