#include "database.h"

#include <QSql>
#include <QSqlQuery>
#include <QSqlError>

#include "debug.h"

using chestnut::Database;

Database::Database(QString file_path)
{
  db_ = QSqlDatabase::addDatabase("QSQLITE");
  db_.setDatabaseName(std::move(file_path));
  Q_ASSERT(db_.open());
  qInfo() << "Opened database, path =" << db_.databaseName();
  setupEffectsTable();
}


std::shared_ptr<Database> Database::instance(const QString& file_path)
{
  if (instance_) {
    return instance_;
  } else if (file_path.length() > 0) {
    instance_ = std::make_shared<Database>(file_path);
    return instance_;
  }
  throw std::runtime_error("No DB instance and no db file-path specified");
}


bool Database::addNewPreset(const EffectPreset& value)
{
  QSqlQuery q(db_);
  const auto e_id = effectId(value.effect_name_);
  Q_ASSERT(e_id >= 0);

  q.prepare("INSERT INTO presets (name, e_id) "
            "VALUES (?, ?)");
  q.addBindValue(value.preset_name_);
  q.addBindValue(e_id);
  if (!q.exec()) {
    qWarning() << q.lastError().text();
  }

  const auto p_id = presetId(value.preset_name_);
  Q_ASSERT(p_id >= 0);

  for (const auto& t : value.parameters_) {
    for (const auto& [row, params] : t.toStdMap()) {
      const auto row_id = effectRowId(row, e_id);
      Q_ASSERT(row_id >= 0);
      for (const auto& param : params) {
        Q_ASSERT(addNewParameterPreset(p_id, row_id, param.first, param.second));
      }
    }
  }

  return true;
}


const QSqlResult* Database::query(const QString& statement)
{
  QSqlQuery q(db_);
  if (!q.exec(statement)) {
    qWarning() << q.lastError().text();
    return {};
  }
  return q.result();
}


int Database::effectId(const QString& name, const bool recurse)
{
  QSqlQuery q(db_);
  q.prepare("SELECT id FROM effects WHERE name=?");
  q.addBindValue(name);
  q.exec();
  if (q.first()) {
    return q.value(0).toInt();
  } else if (recurse) {
    q.prepare("INSERT INTO effects (name) VALUES (?)");
    q.addBindValue(name);
    q.exec();
    return effectId(name, false);
  }
  return -1;
}

int Database::effectRowId(const QString& name, const int effect_id, const bool recurse)
{
  QSqlQuery q(db_);
  q.prepare("SELECT id FROM effect_rows WHERE name=? AND e_id=?");
  q.addBindValue(name);
  q.addBindValue(effect_id);
  q.exec();
  if (q.first()) {
    return q.value(0).toInt();
  } else if (recurse) {
    q.prepare("INSERT INTO effect_rows (name, e_id) VALUES (?, ?)");
    q.addBindValue(name);
    q.addBindValue(effect_id);
    q.exec();
    return effectRowId(name, false);
  }
  return -1;
}

int Database::presetId(const QString& name)
{
  QSqlQuery q(db_);
  q.prepare("SELECT id FROM presets WHERE name=?");
  q.addBindValue(name);
  q.exec();
  if (q.first()) {
    return q.value(0).toInt();
  }
  return -1;
}


bool Database::addNewParameterPreset(const int preset_id, const int row_id, QString name, QVariant value)
{
  QSqlQuery q(db_);
  q.prepare("INSERT INTO preset_parameter"
            "(name, value, p_id, er_id)"
            "VALUES (?, ?, ?, ?)");
  q.addBindValue(name);
  auto v = value.toString();
  q.addBindValue(v);
  q.addBindValue(preset_id);
  q.addBindValue(row_id);
  if (!q.exec()) {
    qWarning() << q.lastError().text();
    return false;
  }
  return true;
}

void Database::setupEffectsTable()
{
  query("CREATE TABLE IF NOT EXISTS effects ("
        "id INTEGER,"
        "name VARCHAR(256),"
        "PRIMARY KEY (id) )");

  query("CREATE TABLE IF NOT EXISTS presets ("
        "id INTEGER,"
        "name VARCHAR(256),"
        "e_id INTEGER,"
        "PRIMARY KEY (id),"
        "FOREIGN KEY (e_id) REFERENCES effects(id) )");

  query("CREATE TABLE IF NOT EXISTS effect_rows ("
        "id INTEGER,"
        "name VARCHAR(256),"
        "e_id INTEGER,"
        "PRIMARY KEY (id),"
        "FOREIGN KEY (e_id) REFERENCES effects(id))");

  query("CREATE TABLE IF NOT EXISTS preset_parameter ("
        "id INTEGER,"
        "name VARCHAR(256),"
        "value VARCHAR(256),"
        "p_id INTEGER,"
        "er_id INTEGER,"
        "PRIMARY KEY (id),"
        "FOREIGN KEY (p_id) REFERENCES presets(id),"
        "FOREIGN KEY (er_id) REFERENCES effect_rows(id))");
}


QString Database::variantAsString(QVariant val) const
{
  const auto v_t = val.type();
  switch (v_t) {
    case QVariant::Bool:
      return val.toBool() ? "true" : "false";
    case QVariant::Int:
      [[fallthrough]];
    case QVariant::UInt:
      [[fallthrough]];
    case QVariant::LongLong:
      [[fallthrough]];
    case QVariant::ULongLong:
      [[fallthrough]];
    case QVariant::Double:
      return QString::number(val.toInt());
    case QVariant::Char:
      return val.toChar();
    case QVariant::String:
      return val.toString();
    default:
      throw std::runtime_error("Unhandled variant type conversion");
  }
}
