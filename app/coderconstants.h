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

/**
 * Various constants for encoders and decoders used project-wide
 */

namespace  {
  static const QStringList MPEG2_LOW_LEVEL_FRATES = {"23.976", "24", "25", "29.97", "30"};
  static const QStringList MPEG2_HIGH1440_LEVEL_FRATES = {"23.976", "24", "25", "29.97", "30", "50", "59.94", "60"};
  static const QStringList DNXHD_440_FRATES = {"29.97", "59.94", "60"};
  static const QStringList DNXHD_365_FRATES = {"25", "50"};
  static const QStringList DNXHD_350_FRATES = {"23.976", "24"};
  static const QStringList DNXHD_290_FRATES = {"29.97", "59.94", "60"};
  static const QStringList DNXHD_220_FRATES = {"29.97", "59.94"};
  static const QStringList DNXHD_185_FRATES = {"25", "29.97", "59.94"};
  static const QStringList DNXHD_145_FRATES = {"29.97"};
  static const QStringList DNXHD_100_FRATES = {"29.97"};
  static const QStringList DNXHD_90_FRATES = {"59.94", "60"};
}

constexpr auto MPEG2_SIMPLE_PROFILE = "SP";
constexpr auto MPEG2_MAIN_PROFILE = "MP";
constexpr auto MPEG2_HIGH_PROFILE = "HP";
constexpr auto MPEG2_422_PROFILE = "422";

constexpr auto MPEG2_LOW_LEVEL = "LL";
constexpr auto MPEG2_MAIN_LEVEL = "ML";
constexpr auto MPEG2_HIGH1440_LEVEL = "H-14";
constexpr auto MPEG2_HIGH_LEVEL = "HL";


static const auto MPEG2_MAIN_LEVEL_FRATES = MPEG2_LOW_LEVEL_FRATES;
static const auto MPEG2_HIGH_LEVEL_FRATES = MPEG2_HIGH1440_LEVEL_FRATES;

constexpr auto X264_MINIMUM_CRF = 0;
constexpr auto X264_MAXIMUM_CRF = 51;


constexpr auto H264_BASELINE_PROFILE = "BP";
constexpr auto H264_EXTENDED_PROFILE = "XP";
constexpr auto H264_MAIN_PROFILE = "MP";
constexpr auto H264_HIGH_PROFILE = "HiP";

static const QStringList MPEG2_PROFILES {MPEG2_SIMPLE_PROFILE, MPEG2_MAIN_PROFILE, MPEG2_HIGH_PROFILE, MPEG2_422_PROFILE};
static const QStringList MPEG2_LEVELS {MPEG2_LOW_LEVEL, MPEG2_MAIN_LEVEL, MPEG2_HIGH1440_LEVEL, MPEG2_HIGH_LEVEL};
static const QStringList H264_PROFILES {H264_BASELINE_PROFILE, H264_EXTENDED_PROFILE, H264_MAIN_PROFILE, H264_HIGH_PROFILE};
static const QStringList FRAME_RATES {"10", "12", "15", "23.976", "24", "25", "29.97", "30", "48", "50", "59.94", "60"};
// TODO: use correct naming convention e.g. DNxHD LB, DNxHD SQ, etc
static const QStringList DNXHD_PROFILES {"440x", "440", "365x", "365", "350x", "350", "220x", "220", "185x", "185", "145",
                                         "100", "90"};

//static const QStringList DNXHD_PROFILES {"440x", "440", "365x", "365", "350x", "350", "290", "240", "220x", "220", "185x",
//                                         "185", "175x", "175", "145", "120", "115", "100", "90x", "90", "85", "75", "50",
//                                         "45", "36"};


struct LevelConstraints
{
    int max_width {-1};
    int max_height {-1};
    int max_bitrate {-1};
    QStringList framerates {};
    QStringList depths {};
    /* false indicates 4:4:4 */
    bool subsampling {true};

    LevelConstraints() = default;

    LevelConstraints(const int w, const int h, const int b, QStringList r)
      : max_width(w),
        max_height(h),
        max_bitrate(b),
        framerates(std::move(r)),
        depths({"8"}),
        subsampling(true)
    {
    }

    LevelConstraints(const int w, const int h, const int b, QStringList r, QStringList d)
      : max_width(w),
        max_height(h),
        max_bitrate(b),
        framerates(std::move(r)),
        depths(std::move(d)),
        subsampling(true)
    {
    }

    LevelConstraints(const int w, const int h, const int b, QStringList r, QStringList d, const bool s)
      : max_width(w),
        max_height(h),
        max_bitrate(b),
        framerates(std::move(r)),
        depths(std::move(d)),
        subsampling(s)
    {
    }
};

static const LevelConstraints MPEG2_LL_CONSTRAINTS {352, 288, 4000000, MPEG2_LOW_LEVEL_FRATES};
static const LevelConstraints MPEG2_ML_CONSTRAINTS {720, 576, 15000000, MPEG2_MAIN_LEVEL_FRATES};
static const LevelConstraints MPEG2_H14_CONSTRAINTS {1440, 1152, 60000000, MPEG2_HIGH1440_LEVEL_FRATES};
static const LevelConstraints MPEG2_HL_CONSTRAINTS {1920, 1080, 80000000, MPEG2_HIGH_LEVEL_FRATES};

static const LevelConstraints H264_20_CONSTRAINTS {-1, -1, 2000000, {}};
static const LevelConstraints H264_21_CONSTRAINTS {-1, -1, 4000000, {}};
static const LevelConstraints H264_22_CONSTRAINTS {-1, -1, 4000000, {}};
static const LevelConstraints H264_30_CONSTRAINTS {-1, -1, 10000000, {}};
static const LevelConstraints H264_31_CONSTRAINTS {-1, -1, 14000000, {}};
static const LevelConstraints H264_32_CONSTRAINTS {-1, -1, 20000000, {}};
static const LevelConstraints H264_40_CONSTRAINTS {-1, -1, 20000000, {}};
static const LevelConstraints H264_41_CONSTRAINTS {-1, -1, 50000000, {}};
static const LevelConstraints H264_42_CONSTRAINTS {-1, -1, 50000000, {}};
static const LevelConstraints H264_50_CONSTRAINTS {-1, -1, 135000000, {}};
static const LevelConstraints H264_51_CONSTRAINTS {-1, -1, 240000000, {}};
static const LevelConstraints H264_52_CONSTRAINTS {-1, -1, 240000000, {}};


static const LevelConstraints DNXHD_440X_CONSTRAINTS {1920, 1080, 440000000, DNXHD_440_FRATES, {"10"}};
static const LevelConstraints DNXHD_440_CONSTRAINTS {1920, 1080, 440000000, DNXHD_440_FRATES};
static const LevelConstraints DNXHD_365X_CONSTRAINTS {1920, 1080, 365000000, DNXHD_365_FRATES, {"10"}}; // can be 4:4:4 but only @25fps
static const LevelConstraints DNXHD_365_CONSTRAINTS {1920, 1080, 365000000, DNXHD_365_FRATES};
static const LevelConstraints DNXHD_350X_CONSTRAINTS {1920, 1080, 350000000, DNXHD_350_FRATES, {"10"}, false};
static const LevelConstraints DNXHD_350_CONSTRAINTS {1920, 1080, 350000000, DNXHD_350_FRATES};
static const LevelConstraints DNXHD_290_CONSTRAINTS {1920, 1080, 290000000, DNXHD_290_FRATES};
static const LevelConstraints DNXHD_220X_CONSTRAINTS {1920, 1080, 220000000, DNXHD_220_FRATES, {"10"}};
static const LevelConstraints DNXHD_220_CONSTRAINTS {1920, 1080, 220000000, DNXHD_220_FRATES};
static const LevelConstraints DNXHD_185X_CONSTRAINTS {1920, 1080, 185000000, DNXHD_185_FRATES, {"10"}};
static const LevelConstraints DNXHD_185_CONSTRAINTS {1920, 1080, 185000000, DNXHD_185_FRATES};
static const LevelConstraints DNXHD_145_CONSTRAINTS {1920, 1080, 145000000, DNXHD_145_FRATES};
static const LevelConstraints DNXHD_100_CONSTRAINTS {1920, 1080, 100000000, DNXHD_100_FRATES};
static const LevelConstraints DNXHD_90_CONSTRAINTS {1920, 1080, 90000000, DNXHD_90_FRATES};


#endif // CODERCONSTANTS_H
