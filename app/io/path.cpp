/* 
 * Olive. Olive is a free non-linear video editor for Windows, macOS, and Linux.
 * Copyright (C) 2018  {{ organization }}
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
 *along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "path.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDir>

#include "debug.h"

constexpr auto PORTABLE_DIR = "/portable";

namespace  {
  QString real_app_dir;
}

QString chestnut::paths::appDir()
{
  if (real_app_dir.isEmpty()) {
    real_app_dir = QCoreApplication::applicationDirPath();
  }
  return real_app_dir;
}

QString chestnut::paths::dataPath()
{
  const QString app_dir(appDir());
  if (QFileInfo::exists(app_dir + PORTABLE_DIR)) {
    return app_dir;
  } else {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  }
}

QString chestnut::paths::configPath()
{
  const QString app_dir(appDir());
  if (QFileInfo::exists(app_dir + PORTABLE_DIR)) {
    return app_dir;
  } else {
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
  }
}
