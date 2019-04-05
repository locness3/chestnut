#ifndef COLORCONVERSIONS_H
#define COLORCONVERSIONS_H

#include <QRgb>

constexpr double LUMA_RED_COEFF = 0.2126;
constexpr double LUMA_GREEN_COEFF = 0.7152;
constexpr double LUMA_BLUE_COEFF = 0.0722;


namespace io
{
  namespace color_conversion
  {
    int32_t rgbToLuma(const QRgb clr);
  }
}

#endif // COLORCONVERSIONS_H
