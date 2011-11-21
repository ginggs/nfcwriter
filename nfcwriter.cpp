#include "nfcwriter.h"

#include <QContactManager>

#include <QContactAvatar>
#include <QContactGuid>
#include <QContactTimestamp>

#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>
#include "ndefnfcmimevcardrecord.h"
#include "ndefnfcsprecord.h"
#include "ndefnfcmimebtrecord.h"

#include <QFile>
#include <QDir>

nfcwriter::nfcwriter(QObject *parent) :
    QObject(parent),
    op(nfcwriter::mode_none),
    text(),
    uri(),
    action(),
    cto(),
    nfc(),
    nfctag() {
    qDebug() << "nfcwriter::nfcwriter";

    connect(&nfc, SIGNAL(targetDetected(QNearFieldTarget*)),
            this, SLOT(targetDetected(QNearFieldTarget*)));
    connect(&nfc, SIGNAL(targetLost(QNearFieldTarget*)),
            this, SLOT(targetLost(QNearFieldTarget*)));
}

bool nfcwriter::check() {
    qDebug() << "nfcwriter::check";
    if (! nfc.isAvailable() ) {
        qDebug() << "nfc not available";
        return false;
    }
    return true;
}

void nfcwriter::read() {
    qDebug() << "nfcwriter::read";

    nfc.setTargetAccessModes(QNearFieldManager::NdefReadTargetAccess);
    nfc.startTargetDetection();
    op = nfcwriter::mode_read;
    emit nfcTap();
}

void nfcwriter::writetext(QString t) {
    qDebug() << "nfcwriter::writetext" << t;
    text = t;

    nfc.setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    nfc.startTargetDetection();
    op = nfcwriter::mode_text;
    emit nfcTap();
}

void nfcwriter::writeuri(QString u) {
    qDebug() << "nfcwriter::writeuri" << u;
    uri = QUrl(u);

    nfc.setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    nfc.startTargetDetection();
    op = nfcwriter::mode_uri;
    emit nfcTap();
}

void nfcwriter::writeid(QContactLocalId id) {
    qDebug() << "nfcwriter::writeid" << id;

    QContactManager mgr;
    cto = mgr.contact(id);
    cto.removeDetail( new QContactDetail(cto.detail<QContactAvatar>()));
    cto.removeDetail( new QContactDetail(cto.detail<QContactGuid>()));
    cto.removeDetail( new QContactDetail(cto.detail<QContactTimestamp>()));
    qDebug() << "cto.id" << cto.id();

    nfc.setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    nfc.startTargetDetection();
    op = nfcwriter::mode_vcard;
    emit nfcTap();
}

void nfcwriter::writesp(QString t, QString u, int a) {
    qDebug() << "nfcwriter::writeusp" << t << u << a;
    text = t;
    uri = QUrl(u);
    action = a;

    nfc.setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    nfc.startTargetDetection();
    op = nfcwriter::mode_smartposter;
    emit nfcTap();
}

void nfcwriter::writebt(QString mac) {
    qDebug() << "nfcwriter::writebt" << mac;
    mac.remove(':');
    nfctag.clear();
    int tmp = 0;

    for (int i=0; i<mac.size(); ++i) {
        switch (mac.at(i).toLower().toAscii() ) {
        case '0':
            tmp += char(0) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case '1':
            tmp += char(1) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case '2':
            tmp += char(2) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case '3':
            tmp += char(3) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case '4':
            tmp += char(4) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case '5':
            tmp += char(5) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case '6':
            tmp += char(6) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case '7':
            tmp += char(7) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case '8':
            tmp += char(8) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case '9':
            tmp += char(9) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case 'a':
            tmp += char(10) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case 'b':
            tmp += char(11) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case 'c':
            tmp += char(12) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case 'd':
            tmp += char(13) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case 'e':
            tmp += char(14) * ( (i%2) ? 0x01 : 0x10 );
            break;
        case 'f':
            tmp += char(15) * ( (i%2) ? 0x01 : 0x10 );
            break;
        }

        if (i%2) nfctag += tmp;
        if (i%2) tmp = 0;
    }
    qDebug() << "MACHEX" << nfctag.toHex();

    nfc.setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    nfc.startTargetDetection();
    op = nfcwriter::mode_bt;
    emit nfcTap();
}


void nfcwriter::clone() {
    qDebug() << "nfcwriter::clone";

    nfc.setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    nfc.startTargetDetection();
    op = nfcwriter::mode_clone;
    emit nfcTap();
}

void nfcwriter::cancel() {
    qDebug() << "nfcwriter::cancel";
    nfc.stopTargetDetection();
    op = nfcwriter::mode_none;
}

void nfcwriter::targetDetected(QNearFieldTarget *target) {
    qDebug() << "nfcwriter::targetDetected" << target;
    QNdefNfcTextRecord ndef_text;
    QNdefNfcUriRecord ndef_uri;
    NdefNfcMimeVcardRecord ndef_vcard;
    NdefNfcSpRecord ndef_sp;
    NdefNfcMimeBtRecord ndef_bt;

    switch (op) {
    case nfcwriter::mode_none:
        qDebug() << "why op == nfcwriter::mode_none ? ";
        break;
    case nfcwriter::mode_read:
        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        if (target->hasNdefMessage()) {
            connect(target, SIGNAL(ndefMessageRead(QNdefMessage)),
                    this, SLOT(ndefMessageRead(QNdefMessage)));
            target->readNdefMessages();
        }

        break;
    case nfcwriter::mode_text:
        ndef_text.setText(text);
        connect(target, SIGNAL(ndefMessagesWritten()),
                this, SLOT(ndefMessageWritten()));

        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        target->writeNdefMessages(QList<QNdefMessage>() << QNdefMessage(ndef_text));
        break;
    case nfcwriter::mode_uri:
        ndef_uri.setUri(uri);
        connect(target, SIGNAL(ndefMessagesWritten()),
                this, SLOT(ndefMessageWritten()));

        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        target->writeNdefMessages(QList<QNdefMessage>() << QNdefMessage(ndef_uri));
        break;
    case nfcwriter::mode_vcard:
        ndef_vcard.setContact(cto);
        connect(target, SIGNAL(ndefMessagesWritten()),
                this, SLOT(ndefMessageWritten()));

        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        target->writeNdefMessages(QList<QNdefMessage>() << QNdefMessage(ndef_vcard));
        break;
    case nfcwriter::mode_smartposter:
        ndef_text.setText(text);
        ndef_uri.setUri(uri);

        ndef_sp.addTitle(ndef_text);
        ndef_sp.setUri(ndef_uri);
        switch (action) {
        case 0: // DoAction
            ndef_sp.setAction(NdefNfcSpRecord::DoAction);
            break;
        case 1: // SaveForLater
            ndef_sp.setAction(NdefNfcSpRecord::SaveForLater);
            break;
        case 2: // OpenForEditing
            ndef_sp.setAction(NdefNfcSpRecord::OpenForEditing);
            break;
        case 3: // RFU
            ndef_sp.setAction(NdefNfcSpRecord::RFU);
            break;
        }
        connect(target, SIGNAL(ndefMessagesWritten()),
                this, SLOT(ndefMessageWritten()));

        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        target->writeNdefMessages(QList<QNdefMessage>() << QNdefMessage(ndef_sp));
        break;
    case nfcwriter::mode_bt:
        connect(target, SIGNAL(ndefMessagesWritten()),
                this, SLOT(ndefMessageWritten()));

        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        ndef_bt.setBtmac(nfctag);
        target->writeNdefMessages(QList<QNdefMessage>() << QNdefMessage(ndef_bt));

        break;
    case nfcwriter::mode_clone:
        connect(target, SIGNAL(ndefMessagesWritten()),
                this, SLOT(ndefMessageWritten()));

        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        target->writeNdefMessages(QList<QNdefMessage>() << QNdefMessage::fromByteArray(nfctag) );

        break;
    }
}

void nfcwriter::targetLost(QNearFieldTarget *target) {
    qDebug() << "nfcwriter::targetLost" << target;
    nfc.stopTargetDetection();
    op = nfcwriter::mode_none;
    target->disconnect();

    emit nfcLost();
}

void nfcwriter::targetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id) {
    Q_UNUSED(id);

    QString errorString = "Unknown";
    switch (error) {
    case QNearFieldTarget::NoError:
        errorString = "NoError";
        break;
    case QNearFieldTarget::UnknownError:
        errorString = "UnknownError";
        break;
    case QNearFieldTarget::UnsupportedError:
        errorString = "UnsupportedError";
        break;
    case QNearFieldTarget::TargetOutOfRangeError:
        errorString = "TargetOutOfRangeError";
        break;
    case QNearFieldTarget::NoResponseError:
        errorString = "NoResponseError";
        break;
    case QNearFieldTarget::ChecksumMismatchError:
        errorString = "ChecksumMismatchError";
        break;
    case QNearFieldTarget::InvalidParametersError:
        errorString = "InvalidParametersError";
        break;
    case QNearFieldTarget::NdefReadError:
        errorString = "NdefReadError";
        break;
    case QNearFieldTarget::NdefWriteError:
        errorString = "NdefWriteError";
        break;
    }
    qDebug() << "nfcwriter::targetError" << error << errorString;
}

void nfcwriter::ndefMessageRead(QtMobility::QNdefMessage msg) {
    qDebug() << "nfcwriter::ndefMessageRead";
    text.clear();
    nfctag = msg.toByteArray();

    QFile tmp(QDir::tempPath() + QDir::separator() + "nfctag.bin");

    if (tmp.open(QIODevice::WriteOnly)) {
        qDebug() << "BINSIZE" << tmp.write(nfctag);
        tmp.close();
    }

    text.append("Records: " + QString("%1").arg(msg.size()) + QString('\n'));
    text.append("Bytes: " + QString("%1").arg(msg.toByteArray().size()) + QString('\n'));

    int i = 1;
    foreach (const QNdefRecord &r, msg) {
        tmp.setFileName(QDir::tempPath() + QDir::separator() + "nfctag-" + QVariant(i).toString());
        if (tmp.open(QIODevice::WriteOnly)) {
            qDebug() << "Record" << i
                     << "Size" << tmp.write(r.payload())
                     << "Type" << r.type();
            tmp.close();
        }
        ++i;

        if (r.isRecordType<QNdefNfcTextRecord>()) {
            text.append("Text: " + QNdefNfcTextRecord(r).text() + QString('\n'));
        } else if (r.isRecordType<QNdefNfcUriRecord>()) {
            text.append("Uri: " + QNdefNfcUriRecord(r).uri().toString() + QString('\n'));
        } else if (r.isRecordType<NdefNfcSpRecord>()) {
            NdefNfcSpRecord sp(r);
            text.append("SmartPoster" + QString('\n'));
            foreach (QNdefNfcTextRecord t, sp.titles()) {
                text.append("Text: " + t.text() + QString('\n'));
            }
            text.append("Uri: " + sp.uri().toString() + QString('\n'));
            switch (sp.action()) {
            case NdefNfcSpRecord::DoAction:
                text.append("Action: DoAction" + QString('\n'));
                break;
            case NdefNfcSpRecord::SaveForLater:
                text.append("Action: SaveForLater" + QString('\n'));
                break;
            case NdefNfcSpRecord::OpenForEditing:
                text.append("Action: OpenForEditing" + QString('\n'));
                break;
            case NdefNfcSpRecord::RFU:
                text.append("Action: RFU" + QString('\n'));
                break;
            }
        } else if (r.isRecordType<NdefNfcMimeVcardRecord>()) {
            text.append("VCard" + QString('\n'));
            foreach (QContact c, NdefNfcMimeVcardRecord(r).contacts()) {
                foreach (QContactDetail d, c.details()) {
                    text.append(d.definitionName() + ": ");
                    foreach (QVariant dd, d.variantValues()) {
                        text.append(dd.toString() + ' ');
                    }
                    text.append(QString('\n'));
                }
            }
        } else {
            text.append("Unknow Record" + QString('\n'));
        }
    }
    emit nfcRead();
}

void nfcwriter::ndefMessageWritten() {
    qDebug() << "nfcwriter::ndefMessageWritten";
    emit nfcWritten();
}
