#include "ndefnfcmimebtrecord.h"
#include <QVariant>

QByteArray NdefNfcMimeBtRecord::btmac() const {
    qDebug() << "NdefNfcMimeBtRecord::btmac";
    QByteArray p = payload();

    QByteArray mac;
    if (p.size() >= 8) for (int i=7; i>1; --i) mac += p.at(i);
    return mac;
}

void NdefNfcMimeBtRecord::setBtmac(QByteArray mac) {
    qDebug() << "NdefNfcMimeBtRecord::setBtmac" << mac;
    if (mac.size()!=6) return;

    QByteArray p;
    for (int i=5; i>=0; --i) p += mac.at(i);

    p += QByteArray::fromHex("04 0D 3C 04 6C"); // audio

    p.prepend(p.size()+2);
    p.prepend((p.size()+2 > 255) ? p.size()+2-255 : char(0) );

    setPayload(p);
    qDebug() << "Playload HEX" << p.toHex();
}
