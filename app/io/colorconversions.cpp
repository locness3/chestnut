#include "colorconversions.h"

#include <cmath>


int32_t io::color_conversion::rgbToLuma(const QRgb clr)
{
  return static_cast<int32_t>(lround(qRed(clr) * LUMA_RED_COEFF)
                              + lround(qGreen(clr) * LUMA_GREEN_COEFF)
                              + lround(qBlue(clr) * LUMA_BLUE_COEFF));
}
