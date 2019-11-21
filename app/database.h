#ifndef DATABASE_H
#define DATABASE_H

#include <memory>
#include <QSqlDatabase>
#include <QVariant>
#include <QSqlResult>

namespace chestnut
{
  struct EffectPreset
  {
    QString effect_name_;
    QString preset_name_;
    QVector<QPair<QString, QVariant>> parameters_;
  };

  class Database
  {
    public:
      explicit Database(QString file_path);
      static std::shared_ptr<Database> instance(const QString& file_path="");

      bool addNewPreset(const EffectPreset& value);

    private:
      QSqlDatabase db_;
      inline static std::shared_ptr<Database> instance_{nullptr};

      void setupEffectsTable();

      const QSqlResult* query(const QString& statement);

      int effectId(const QString& name, const bool recurse=true);
  };
}

#endif // DATABASE_H
