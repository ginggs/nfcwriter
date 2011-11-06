#ifndef NFCWRITER_H
#define NFCWRITER_H

#include <QObject>

#include <QString>
#include <QUrl>
#include <QContact>
#include <QNearFieldManager>

#include <QNdefMessage>

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
    void nfcRead();

public slots:
    bool check();
    void read();
    void writetext(QString text);
    void writeuri(QString uri);
    void writeid(QContactLocalId id);
    void writesp(QString text, QString uri, int action);
    QString get() { return tag; }
    void dump();
    void burn();

private slots:
    void targetDetected(QNearFieldTarget *target);
    void targetLost(QNearFieldTarget *target);
    void targetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);
    void requestCompleted(QNearFieldTarget::RequestId id);
    void ndefMessageRead(QNdefMessage msg);
    void ndefMessageDump(QNdefMessage msg);
    void ndefMessageWritten();

private:
    // mode
    enum menu {
        mode_none,
        mode_read,
        mode_text,
        mode_uri,
        mode_vcard,
        mode_smartposter,
        mode_dump,
        mode_burn
    };
    menu op;
    // Text
    QString text;
    // Uri
    QUrl uri;
    // SP = Text + Uri + Action
    int action;
    // Vcard
    QContact cto;
    // NFC
    QNearFieldManager nfc;
    // TAG
    QString tag;
    // MSG
    QByteArray nfctag;
};

#endif // NFCWRITER_H
