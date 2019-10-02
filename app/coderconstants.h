/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2019
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CODERCONSTANTS_H
#define CODERCONSTANTS_H

#include <QStringList>
#include <optional>


/**
 * Various constants for encoders and decoders used project-wide
 */

namespace  {
  static const QStringList MPEG2_LOW_LEVEL_FRATES = {"23.976", "24", "25", "29.97", "30"};
  static const QStringList MPEG2_HIGH1440_LEVEL_FRATES = {"23.976", "24", "25", "29.97", "30", "50", "59.94", "60"};
  static const QStringList MPEG4_SSTP_1_5_FRATES = {"23.976", "24", "25", "29.97", "30"};
  static const QStringList MPEG4_SSTP_6_FRATES = {"23.976", "24", "25", "29.97", "30", "48", "50", "59.94", "60"};
  static const QStringList DNXHD_440_FRATES = {"29.97", "59.94", "60"};
  static const QStringList DNXHD_365_FRATES = {"25", "50"};
  static const QStringList DNXHD_350_FRATES = {"23.976", "24"};
  static const QStringList DNXHD_290_FRATES = {"29.97", "59.94", "60"};
  static const QStringList DNXHD_220_FRATES = {"29.97", "59.94"};
  static const QStringList DNXHD_185_FRATES = {"25", "29.97", "59.94"};
  static const QStringList DNXHD_145_FRATES = {"29.97"};
  static const QStringList DNXHD_90_FRATES = {"59.94", "60"};
}

static const QStringList ALL_FRATES = {"10", "12", "15", "23.976", "24", "25", "29.97", "30", "48", "50", "59.94", "60"};
constexpr auto DEPTH_8 = "8";
constexpr auto DEPTH_10 = "10";
constexpr auto DEPTH_12 = "12";

constexpr auto MPEG2_SIMPLE_PROFILE = "SP";
constexpr auto MPEG2_MAIN_PROFILE = "MP";
constexpr auto MPEG2_HIGH_PROFILE = "HP";
constexpr auto MPEG2_422_PROFILE = "422";

constexpr auto MPEG2_LOW_LEVEL = "LL";
constexpr auto MPEG2_MAIN_LEVEL = "ML";
constexpr auto MPEG2_HIGH1440_LEVEL = "H-14";
constexpr auto MPEG2_HIGH_LEVEL = "HL";

// MPEG4-part2
constexpr auto MPEG4_SSTP_PROFILE = "SStP";
constexpr auto MPEG4_SSTP_1_LEVEL = "1";
constexpr auto MPEG4_SSTP_2_LEVEL = "2";
constexpr auto MPEG4_SSTP_3_LEVEL = "3";
constexpr auto MPEG4_SSTP_4_LEVEL = "4";
constexpr auto MPEG4_SSTP_5_LEVEL = "5";
constexpr auto MPEG4_SSTP_6_LEVEL = "6";

static const auto MPEG2_MAIN_LEVEL_FRATES = MPEG2_LOW_LEVEL_FRATES;
static const auto MPEG2_HIGH_LEVEL_FRATES = MPEG2_HIGH1440_LEVEL_FRATES;

constexpr auto X264_MINIMUM_CRF = 0;
constexpr auto X264_MAXIMUM_CRF = 51;

// AVC/MPEG4-part10 profiles
constexpr auto H264_BASELINE_PROFILE = "BP";
constexpr auto H264_MAIN_PROFILE = "MP";
constexpr auto H264_HIGH_PROFILE = "HiP";
constexpr auto H264_HIGH10_PROFILE = "Hi10P";
constexpr auto H264_HIGH422_PROFILE = "Hi422P";
constexpr auto H264_HIGH444_PROFILE = "Hi444PP";


static const QStringList MPEG2_PROFILES {MPEG2_SIMPLE_PROFILE, MPEG2_MAIN_PROFILE, MPEG2_HIGH_PROFILE, MPEG2_422_PROFILE};
static const QStringList MPEG2_LEVELS {MPEG2_LOW_LEVEL, MPEG2_MAIN_LEVEL, MPEG2_HIGH1440_LEVEL, MPEG2_HIGH_LEVEL};
// not attempting support for anything less than SStP
static const QStringList MPEG4_PROFILES {MPEG4_SSTP_PROFILE};
static const QStringList MPEG4_LEVELS {MPEG4_SSTP_1_LEVEL, MPEG4_SSTP_2_LEVEL, MPEG4_SSTP_3_LEVEL, MPEG4_SSTP_4_LEVEL,
      MPEG4_SSTP_5_LEVEL, MPEG4_SSTP_6_LEVEL};
static const QStringList H264_PROFILES {H264_BASELINE_PROFILE, H264_MAIN_PROFILE, H264_HIGH_PROFILE, H264_HIGH10_PROFILE, H264_HIGH422_PROFILE, H264_HIGH444_PROFILE};
static const QStringList FRAME_RATES {"10", "12", "15", "23.976", "24", "25", "29.97", "30", "48", "50", "59.94", "60"};
// TODO: use correct naming convention e.g. DNxHD LB, DNxHD SQ, etc
static const QStringList DNXHD_PROFILES {"440x", "440", "365x", "365", "350x", "350", "220x", "220", "185x", "185", "145",
                                         "90"};


enum class PixelFormat
{
  YUV420,
  YUV422,
  YUV444
};

using PixFmtList = std::vector<PixelFormat>;

struct Constraints
{
    std::optional<int> max_width;
    std::optional<int> max_height;
    std::optional<int64_t> max_bitrate;
    std::optional<QStringList> framerates;
    std::optional<QStringList> depths;
    /* false indicates 4:4:4 */
    std::optional<PixFmtList> pix_fmts;

    Constraints() = default;

    Constraints(const int w, const int h, const int64_t b, QStringList r, QStringList d)
      : max_width(w),
        max_height(h),
        max_bitrate(b),
        framerates(std::move(r)),
        depths(std::move(d))
    {
    }

    Constraints(const int w, const int h, const int64_t b, QStringList r, QStringList d, std::vector<PixelFormat> formats)
      : max_width(w),
        max_height(h),
        max_bitrate(b),
        framerates(std::move(r)),
        depths(std::move(d)),
        pix_fmts(formats)
    {
    }
};

// MPEG2 Profile constraints
static const Constraints MPEG2_SP_CONSTRAINST{-1, -1, -1, {}, {}, {PixelFormat::YUV420}};

// MPEG2 Level constraints
static const Constraints MPEG2_LL_CONSTRAINTS {352, 288, 4000000, MPEG2_LOW_LEVEL_FRATES, {}};
static const Constraints MPEG2_ML_CONSTRAINTS {720, 576, 15000000, MPEG2_MAIN_LEVEL_FRATES, {}};
static const Constraints MPEG2_H14_CONSTRAINTS {1440, 1152, 60000000, MPEG2_HIGH1440_LEVEL_FRATES, {}};
static const Constraints MPEG2_HL_CONSTRAINTS {1920, 1080, 80000000, MPEG2_HIGH_LEVEL_FRATES, {}};

// MPEG4-part2 SSTP level constraints
static const Constraints MPEG4_SSTP_1_CONSTRAINTS {720, 576, 180000000, MPEG4_SSTP_1_5_FRATES, {DEPTH_8, DEPTH_10}, {PixelFormat::YUV422}}; //TODO: check NTSC/PAL SDTV
static const Constraints MPEG4_SSTP_2_CONSTRAINTS {1920, 1080, 600000000, MPEG4_SSTP_1_5_FRATES, {DEPTH_8, DEPTH_10}, {PixelFormat::YUV422}};
static const Constraints MPEG4_SSTP_3_CONSTRAINTS {1920, 1080, 900000000, MPEG4_SSTP_1_5_FRATES, {DEPTH_8, DEPTH_10, DEPTH_12}, {PixelFormat::YUV444}};
static const Constraints MPEG4_SSTP_4_CONSTRAINTS {2048, 2048, 1350000000, MPEG4_SSTP_1_5_FRATES, {DEPTH_8, DEPTH_10, DEPTH_12}, {PixelFormat::YUV444}};
static const Constraints MPEG4_SSTP_5_CONSTRAINTS {3840, 2048, 1800000000, MPEG4_SSTP_1_5_FRATES, {DEPTH_8, DEPTH_10, DEPTH_12}, {PixelFormat::YUV444}};
static const Constraints MPEG4_SSTP_6_CONSTRAINTS {3840, 2048, 3600000000, MPEG4_SSTP_6_FRATES, {DEPTH_8, DEPTH_10, DEPTH_12}, {PixelFormat::YUV444}};

static const Constraints H264_20_CONSTRAINTS {-1, -1, 2000000, {}, {}};
static const Constraints H264_21_CONSTRAINTS {-1, -1, 4000000, {}, {}};
static const Constraints H264_22_CONSTRAINTS {-1, -1, 4000000, {}, {}};
static const Constraints H264_30_CONSTRAINTS {-1, -1, 10000000, {}, {}};
static const Constraints H264_31_CONSTRAINTS {-1, -1, 14000000, {}, {}};
static const Constraints H264_32_CONSTRAINTS {-1, -1, 20000000, {}, {}};
static const Constraints H264_40_CONSTRAINTS {-1, -1, 20000000, {}, {}};
static const Constraints H264_41_CONSTRAINTS {-1, -1, 50000000, {}, {}};
static const Constraints H264_42_CONSTRAINTS {-1, -1, 50000000, {}, {}};
static const Constraints H264_50_CONSTRAINTS {-1, -1, 135000000, {}, {}};
static const Constraints H264_51_CONSTRAINTS {-1, -1, 240000000, {}, {}};
static const Constraints H264_52_CONSTRAINTS {-1, -1, 240000000, {}, {}};



static const Constraints DNXHD_440X_CONSTRAINTS {1920, 1080, 440000000, DNXHD_440_FRATES, {DEPTH_10}};
static const Constraints DNXHD_440_CONSTRAINTS {1920, 1080, 440000000, DNXHD_440_FRATES, {}};
static const Constraints DNXHD_365X_CONSTRAINTS {1920, 1080, 365000000, DNXHD_365_FRATES, {DEPTH_10}}; // can be 4:4:4 but only @25fps
static const Constraints DNXHD_365_CONSTRAINTS {1920, 1080, 365000000, DNXHD_365_FRATES, {}};
static const Constraints DNXHD_350X_CONSTRAINTS {1920, 1080, 350000000, DNXHD_350_FRATES, {DEPTH_10}, {PixelFormat::YUV444}};
static const Constraints DNXHD_350_CONSTRAINTS {1920, 1080, 350000000, DNXHD_350_FRATES, {}};
static const Constraints DNXHD_290_CONSTRAINTS {1920, 1080, 290000000, DNXHD_290_FRATES, {}};
static const Constraints DNXHD_220X_CONSTRAINTS {1920, 1080, 220000000, DNXHD_220_FRATES, {DEPTH_10}};
static const Constraints DNXHD_220_CONSTRAINTS {1920, 1080, 220000000, DNXHD_220_FRATES, {}};
static const Constraints DNXHD_185X_CONSTRAINTS {1920, 1080, 185000000, DNXHD_185_FRATES, {DEPTH_10}};
static const Constraints DNXHD_185_CONSTRAINTS {1920, 1080, 185000000, DNXHD_185_FRATES, {}};
static const Constraints DNXHD_145_CONSTRAINTS {1920, 1080, 145000000, DNXHD_145_FRATES, {}};
static const Constraints DNXHD_90_CONSTRAINTS {1920, 1080, 90000000, DNXHD_90_FRATES, {}};


#endif // CODERCONSTANTS_H
