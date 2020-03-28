#pragma once

#include "settings.h"

#include <QTableWidget>

class QStringListModel;

class SubstitutionsTable : public QTableWidget
{
  Q_OBJECT
public:
  enum class Column { Language = 0, Source, Target, Count };

  explicit SubstitutionsTable(QWidget* parent = nullptr);

  void setSourceLanguageModel(QStringListModel* model);

  void setSubstitutions(const Substitutions& substitutions);
  Substitutions substitutions() const;

private:
  void handleItemChange(QTableWidgetItem* item);
  void addRow(const LanguageId& language = {},
              const Substitution& substutution = {});
  std::pair<LanguageId, Substitution> at(int row) const;
  void updateModel(const Substitutions& substitutions);

  QStringListModel* sourceLanguages_{nullptr};
  QStringListModel* substitutionLanguages_;
};
