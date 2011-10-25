#ifndef NFCWRITER_H
#define NFCWRITER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QContactManager>
#include <QContact>
#include <QNearFieldManager>
#include <QDebug>

using namespace QtMobility;

class nfcwriter : public QObject {
    Q_OBJECT
public:
    explicit nfcwriter(QObject *parent = 0);

signals:
    void nfcTap();
    void nfcWritten();
    void nfcLost();

public slots:
    bool check();
    void read();
    void writetext(QString text);
    void writeuri(QString uri);
    void writeid(QContactLocalId id);
    void writesp(QString text, QString uri, int action);

private slots:
    void targetDetected(QNearFieldTarget *target);
    void targetLost(QNearFieldTarget *target);
    void targetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);
    void ndefMessageWritten();

private:
    // mode
    enum menu {
        mode_none,
        mode_read,
        mode_text,
        mode_uri,
        mode_vcard,
        mode_smartposter
    };
    menu op;
    // Text
    QString text;
    // Uri
    QUrl uri;
    // SP = Text + Uri + Action
    int action;
    // Vcard
    QContactManager mgr;
    QContact cto;
    // NFC
    QNearFieldManager nfc;

};

#endif // NFCWRITER_H
