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
  q.prepare("INSERT INTO presets (name, e_id) "
            "VALUES (?, ?)");
  q.addBindValue(value.preset_name_);
  q.addBindValue(effectId(value.effect_name_));
  if (!q.exec()) {
    qWarning() << q.lastError().text();
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

void Database::setupEffectsTable()
{
  query("CREATE TABLE IF NOT EXISTS effects ("
        "id INTEGER,"
        "name VARCHAR(32),"
        "PRIMARY KEY (id) )");

  query("CREATE TABLE IF NOT EXISTS presets ("
        "id INTEGER,"
        "name VARCHAR(32),"
        "e_id INTEGER,"
        "PRIMARY KEY (id),"
        "FOREIGN KEY (e_id) REFERENCES effects(id) )");

  query("CREATE TABLE IF NOT EXISTS preset_parameter ("
        "id INTEGER,"
        "name VARCHAR(32),"
        "value VARCHAR(32),"
        "p_id INTEGER,"
        "PRIMARY KEY (id),"
        "FOREIGN KEY (p_id) REFERENCES presets(id))");
}
