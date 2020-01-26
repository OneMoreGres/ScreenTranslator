#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QJsonObject>
#include <QNetworkAccessManager>

/*!
 * \brief The Updater class.
 *
 * Allows to download and copy files from remote source to local machine.
 */
class Updater : public QObject {
  Q_OBJECT

  public:
    enum UpdateType {
      UpdateTypeNever, UpdateTypeDaily, UpdateTypeWeekly, UpdateTypeMonthly
    };

    explicit Updater (QObject *parent = 0);

    QString currentAppVersion () const;

    //! Initiate updates check.
    void checkForUpdates ();

    //! Get nearest update check time based on given settings.
    QDateTime nextCheckTime (const QDateTime &lastCheckTime, int updateType) const;

  signals:
    void error (const QString &message);
    //! Emited after all components updated.
    void updated ();

  private slots:
    //! Handle remote downloads finish.
    void replyFinished (QNetworkReply *reply);

  private:
    //! Load current version info (built-in).
    void getCurrentVersion ();
    //! Update current version info with information about preformed updates.
    void updateCurrentVersion ();
    //! Load latest available version info from remote source.
    void getAvailableVersion ();
    //! Check is updates available, prompt user and start update.
    void parseAvailableVersion ();
    //! Start update of given component.
    void getComponent (const QString &component);
    //! Finalize update of given component with given new content.
    void installComponent (const QString &component, const QByteArray &newContent);
    //! Save information about component update on disk (for updateCurrentVersion()).
    void updateVersionInfo (const QString &component, int version);

  private:
    QNetworkAccessManager *network_;
    QJsonObject availableVersion_;
    QJsonObject currentVersion_;
    QString updatesFileName_;
    int componentsUpdating_;
    QString backupSuffix_;
};

#endif // UPDATER_H
