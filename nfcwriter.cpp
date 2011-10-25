#include "nfcwriter.h"

#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>
#include "ndefnfcmimevcardrecord.h"
#include "ndefnfcsprecord.h"

#include <QContactAvatar>
#include <QContactGuid>
#include <QContactTimestamp>

nfcwriter::nfcwriter(QObject *parent) :
    QObject(parent),
    op(nfcwriter::mode_none),
    text(),
    uri(),
    action(),
    mgr(),
    cto(),
    nfc() {
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

    cto = mgr.contact(id);
    cto.removeDetail( new QContactDetail(cto.detail<QContactAvatar>()));
    cto.removeDetail( new QContactDetail(cto.detail<QContactGuid>()));
    cto.removeDetail( new QContactDetail(cto.detail<QContactTimestamp>()));
    qDebug() << cto.id();

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

void nfcwriter::targetDetected(QNearFieldTarget *target) {
    qDebug() << "nfcwriter::targetDetected" << target;
    QNdefNfcTextRecord ndef_text;
    QNdefNfcUriRecord ndef_uri;
    NdefNfcMimeVcardRecord ndef_vcard;
    NdefNfcSpRecord ndef_sp;

    switch (op) {
    case nfcwriter::mode_none:
        qDebug() << "why op == nfcwriter::mode_none?";
        break;
    case nfcwriter::mode_read:
        qDebug() << "not implemented";
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
        errorString = "No error has occurred.";
        break;
    case QNearFieldTarget::UnknownError:
        errorString = "No unknown error has occurred.";
        break;
    case QNearFieldTarget::UnsupportedError:
        errorString = "The requested operation is unsupported by this near field target.";
        break;
    case QNearFieldTarget::TargetOutOfRangeError:
        errorString = "The target is no longer within range.";
        break;
    case QNearFieldTarget::NoResponseError:
        errorString = "The target did not respond.";
        break;
    case QNearFieldTarget::ChecksumMismatchError:
        errorString = "The checksum has detected a corrupted response.";
        break;
    case QNearFieldTarget::InvalidParametersError:
        errorString = "Invalid parameters were passed to a tag type specific function.";
        break;
    case QNearFieldTarget::NdefReadError:
        errorString = "Failed to read NDEF messages from the target.";
        break;
    case QNearFieldTarget::NdefWriteError:
        errorString = "Failed to write NDEF messages to the target.";
        break;
    }
    qDebug() << "nfcwriter::targetError" << error << errorString;
}

void nfcwriter::ndefMessageWritten() {
    qDebug() << "nfcwriter::ndefMessageWritten";
    emit nfcWritten();
}
