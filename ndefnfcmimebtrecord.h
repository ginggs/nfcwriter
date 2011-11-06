#ifndef NDEFNFCMIMEBTRECORD_H
#define NDEFNFCMIMEBTRECORD_H

#include <QNdefRecord>
#include <QNdefMessage>
#include <QDebug>

QTM_USE_NAMESPACE

class NdefNfcMimeBtRecord : public QNdefRecord {
    public:
    Q_DECLARE_NDEF_RECORD(NdefNfcMimeBtRecord,
                          QNdefRecord::Mime,
                          "application/vnd.bluetooth.ep.oob",
                          QByteArray(sizeof(char), char(0)))

    QByteArray btmac() const;
    void setBtmac(QByteArray mac);
};

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(NdefNfcMimeBtRecord,
                                       QNdefRecord::Mime,
                                       "application/vnd.bluetooth.ep.oob")

#endif // NDEFNFCMIMEBTRECORD_H
