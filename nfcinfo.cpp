/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Andreas Jakl (andreas.jakl@nokia.com)
**
** This file is part of an NFC example for Qt Mobility.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include "nfcinfo.h"

NfcInfo::NfcInfo(QObject *parent) :
    QObject(parent),
    pendingWriteNdef(false)
{
}

/*!
  \brief Initialize the NFC access for NDEF targets and start target detection.

  This method emits nfcStatusUpdate signals containing the textual results of
  the operation.

  \return true if starting target detection was successful.
  */
bool NfcInfo::initAndStartNfc()
{
    // NdefManager (this) is the parent; will automaically delete nfcManager
    nfcManager = new QNearFieldManager(this);

    // isAvailable() doesn't work (= missing) with QtM 1.2 beta
    // Works for later versions.
#ifdef MEEGO_EDITION_HARMATTAN
    const bool nfcAvailable = nfcManager->isAvailable();
    if (nfcAvailable) {
        emit nfcStatusUpdate("QtM reports: NFC is available");
    } else {
        emit nfcStatusError("QtM reports: NFC is not available");
    }
#endif

    // MeeGo Harmattan PR 1.0 only allows one target access mode to be active at the same time
#ifdef MEEGO_EDITION_HARMATTAN
    nfcManager->setTargetAccessModes(QNearFieldManager::NdefReadTargetAccess);
#else
    nfcManager->setTargetAccessModes(QNearFieldManager::NdefReadTargetAccess | QNearFieldManager::NdefWriteTargetAccess);
#endif

    // Get notified when the tag gets out of range
    connect(nfcManager, SIGNAL(targetLost(QNearFieldTarget*)),
            this, SLOT(targetLost(QNearFieldTarget*)));
    connect(nfcManager, SIGNAL(targetDetected(QNearFieldTarget*)),
            this, SLOT(targetDetected(QNearFieldTarget*)));

    // Start detecting targets
    bool activationSuccessful = nfcManager->startTargetDetection();
    if (activationSuccessful) {
        emit nfcStatusUpdate("Successfully started target detection");
        return true;
    } else {
        emit nfcStatusError("Error starting NFC target detection");
        return false;
    }
}

/*!
  \brief Check if NFC is supported by the device and if it
  is activated.

  Note: currently, this is only implemented for Symbian using
  nfcsettings component (which uses native Symbian code to query
  the NFC status).

  \return true if and only if NFC is available and activated on Symbian.
  The nfcStatusError / nfcStatusUpdate signals contain more information
  about the actual status (e.g., if the device would support NFC but
  the user needs to activate it).
  */
bool NfcInfo::checkNfcStatus()
{
#ifdef Q_OS_SYMBIAN
    // Construct a new instance.
    nfcSettings = new NfcSettings(this);

    // Retrieve the NFC feature support information.
    NfcSettings::NfcFeature nfcFeature = nfcSettings->nfcFeature();

    if (nfcFeature == NfcSettings::NfcFeatureSupported) {
        // Connect signals for receiving mode change and error notifications.
        connect(nfcSettings, SIGNAL(nfcModeChanged(NfcSettings::NfcMode)), SLOT(handleNfcModeChange(NfcSettings::NfcMode)));
        connect(nfcSettings, SIGNAL(nfcErrorOccurred(NfcSettings::NfcError, int)), SLOT(handleNfcError(NfcSettings::NfcError, int)));

        // Retrieve the initial value of the NFC mode setting.
        NfcSettings::NfcMode nfcMode = nfcSettings->nfcMode();

        if (nfcMode != NfcSettings::NfcModeOn) {
            // NFC is supported but not switched on, prompt the user to enable it.
            emit nfcStatusError(tr("NFC hardware is available but currently switched off."));
            return false;
        } else {
            emit nfcStatusUpdate(tr("NFC is supported and switched on."));
            return true;
        }
    }
    else if (nfcFeature == NfcSettings::NfcFeatureSupportedViaFirmwareUpdate) {
        // Display message to user to update device firmware
        emit nfcStatusError(tr("Update device firmware to enable NFC support."));
        return false;
    } else {
        // Display message informing the user that NFC is not supported by this device.
        emit nfcStatusError(tr("NFC not supported by this device."));
        return false;
    }
#endif
    return false;
}

/*!
  Set the image cache to use for storing images retrieved
  from tags.
  \a tagImageCache instance of the image cache. This
  class will not take ownership of the instance!
  */
void NfcInfo::setImageCache(TagImageCache *tagImageCache)
{
    // Not owned by this class
    imgCache = tagImageCache;
}

/*!
  \brief Called by the NearFieldManager whenever it finds a target.

  This method will create connections between the target and this class
  to be informed about its status. It also attempts to analyze the target
  and emits information through nfcStatusUpdate signals. If a write operation
  is pending, it will be written to the tag. Otherwise, the tag contents will be
  read (if possible).
  */
void NfcInfo::targetDetected(QNearFieldTarget *target)
{
    // Handle potential errors emitted by the target
    connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
            this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));
    connect(target, SIGNAL(requestCompleted (const QNearFieldTarget::RequestId)),
            this, SLOT(requestCompleted(QNearFieldTarget::RequestId)));
    connect(target, SIGNAL(ndefMessagesWritten()),
            this, SLOT(ndefMessageWritten()));

    // Cache the target in any case for future writing
    // (so that we can also write on tags that are empty as of now)
    cachedTarget = target;

    // Check if the target includes a NDEF message
    const bool targetHasNdefMessage = target->hasNdefMessage();
    if (targetHasNdefMessage)
    {
        emit nfcStatusUpdate("NDEF target detected");
    } else {
        emit nfcStatusUpdate("Target detected");
    }

    // Analyze the target and send the info to the UI
    emit nfcInfoUpdate(analyzeTarget(target));

    // Check if we have NDEF access and can read or write to the tag
    QNearFieldTarget::AccessMethods accessMethods = target->accessMethods();
    if (accessMethods.testFlag(QNearFieldTarget::NdefAccess))
    {
        // Is a write operation pending?
        if (!pendingWriteNdef)
        {
            // NO write operation pending, so read the tag if possible
            // If target has an NDEF message...
            if (targetHasNdefMessage)
            {
                // Target has NDEF messages: read them (asynchronous)
                connect(target, SIGNAL(ndefMessageRead(QNdefMessage)),
                        this, SLOT(ndefMessageRead(QNdefMessage)));
#ifdef MEEGO_EDITION_HARMATTAN
                cachedRequestId = target->readNdefMessages();
#else
                // QtM 1.2 beta doesn't return the request id.
                target->readNdefMessages();
#endif
            }
        }
        else
        {
            // Write operation is pending, so attempt writing the message.
#ifdef MEEGO_EDITION_HARMATTAN
            nfcManager->setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
#endif
            // Write a cached NDEF message to the tag
            writeCachedNdefMessage();
        }
    }

}

/*!
  \brief Create a string containing a textual description of the generic
  tag properties.

  \param target the NFC target to analyze. Works for all supported targets,
  not only for NDEF targets.
  */
QString NfcInfo::analyzeTarget(QNearFieldTarget* target)
{
    QString nfcInfo;

    // Tag type
    nfcInfo.append("Type: " + convertTagTypeToString(target->type()) + "\n");
    // Tag UID
    QString uidString = QVariant(target->uid().toHex()).toString();
    nfcInfo.append("UID: " + uidString + "\n");
    // Tag URL (not to be confused with the URL of an NDEF record)
    if (!target->url().isEmpty()) { nfcInfo.append("Url: " + target->url().toString()) + "\n"; }

    // Test the access methods to this target
    QNearFieldTarget::AccessMethods accessMethods = target->accessMethods();
    nfcInfo.append("Access methods: ");
    if (accessMethods.testFlag(QNearFieldTarget::NdefAccess)) {
        // The target supports NDEF records.
        nfcInfo.append("Ndef access\n");
    }
    if (accessMethods.testFlag(QNearFieldTarget::TagTypeSpecificAccess)) {
        // The target supports sending tag type specific commands.
        nfcInfo.append("Tag type specific access\n");
    }
    if (accessMethods.testFlag(QNearFieldTarget::LlcpAccess)) {
        // The target supports peer-to-peer LLCP communication.
        nfcInfo.append("Llcp access\n");
    }

    // Read tag-type specific data
    if (target->type() == QNearFieldTarget::NfcTagType1)
    {
        // NFC Forum Tag Type 1
        QNearFieldTagType1* targetSpecific = qobject_cast<QNearFieldTagType1 *>(target);
        // Tag version
        quint8 tagVersion = targetSpecific->version();
        int tagMajorVersion = tagVersion >> 4;
        int tagMinorVersion = tagVersion & 0x0F;
        nfcInfo.append("Version: " + QString::number(tagMajorVersion) + "." + QString::number(tagMinorVersion));
        // Read all
//        if (!pendingWriteNdef)
//        {
//            cachedRequestId = targetSpecific->readAll();
//        }
    }
    return nfcInfo;
}

/*!
  \brief Emits the nfcTagContents containing a textual description of the
  contents of the NDEF message.

  In case pictures are found, these are added to the image cache and the
  nfcTagImage signal is emitted together with the image ID.
  \param message the NDEF message to analyze.
  */
void NfcInfo::ndefMessageRead(const QNdefMessage &message)
{
    if (message.isEmpty())
    {
        emit nfcTagContents("No records in the Ndef message");
        return;
    }

    QString tagContents;

    QByteArray rawMessage = message.toByteArray();
    const int msgSize = rawMessage.size();
    if (msgSize > 0) {
        tagContents.append("Message size: " + QString::number(msgSize) + " bytes\n");
    }

    const int recordCount = message.size();
    int numRecord = 1;
    // Go through all records in the message
    foreach (const QNdefRecord &record, message)
    {
        if (recordCount > 1)
        {
            tagContents.append("Record " + QString::number(numRecord) + "/" + QString::number(recordCount) + "\n");
        }
        // Print generic information about the record
        tagContents.append("Record type: " + QString(record.type()) + "\n");
        tagContents.append("Type name: " + convertRecordTypeNameToString(record.typeNameFormat()) + "\n");

        // Parse tag contents
        if (record.isRecordType<QNdefNfcUriRecord>())
        {
            // ------------------------------------------------
            // URI
            tagContents.append(parseUriRecord(QNdefNfcUriRecord(record)));
        }
        else if (record.isRecordType<QNdefNfcTextRecord>())
        {
            // ------------------------------------------------
            // Text
            tagContents.append(parseTextRecord(QNdefNfcTextRecord(record)));
        }
        else if (record.isRecordType<NdefNfcSpRecord>())
        {
            // ------------------------------------------------
            // Smart Poster (urn:nfc:wkt:Sp)
            tagContents.append(parseSpRecord(NdefNfcSpRecord(record)));
        }
        else if (record.typeNameFormat() == QNdefRecord::Mime &&
                   record.type().startsWith("image/"))
        {
            // The NdefNfcMimeImageRecord class handles many different
            // record types. To simplify the use, there is no extra
            // class for each individual image type (image/png, gif,
            // jpg, jpeg, etc.)
            // Therefore, the isRecordType<>() method can't be used
            // with the generic image handler class.
            // ------------------------------------------------
            // Image (any supported type)
            tagContents.append(parseImageRecord(NdefNfcMimeImageRecord(record)));
        }
        else if (record.isRecordType<NdefNfcMimeVcardRecord>())
        {
            // ------------------------------------------------
            // Mime type: vCard
            NdefNfcMimeVcardRecord vCardRecord(record);
            tagContents.append(parseVcardRecord(vCardRecord));
        }
        else if (record.isEmpty())
        {
            // ------------------------------------------------
            // Empty
            tagContents.append("[Empty record]\n");
        }
        else
        {
            // ------------------------------------------------
            // Record type not handled by this application
            tagContents.append("[Unknown record: " + record.type() + "]\n");
        }
        numRecord++;
    }
    emit nfcTagContents(tagContents);
}

/*!
  \brief Create a textual description of the contents of the
  Uri (U) record.

  \param record the record to analyze
  \return plain text description of the record contents.
  */
QString NfcInfo::parseUriRecord(const QNdefNfcUriRecord& record)
{
    QString tagContents("[URI]\n");
    tagContents.append(record.uri().toString());
    return tagContents;
}

/*!
  \brief Create a textual description of the contents of the
  Text (T) record.

  \param record the record to analyze
  \return plain text description of the record contents.
  */
QString NfcInfo::parseTextRecord(const QNdefNfcTextRecord& record)
{
    QString tagContents("[Text]\n");
    // Emit a signal with the text
    tagContents.append(textRecordToString(record));
    return tagContents;
}

/*!
  \brief Convert the details of a text record to a string.

  Seperated from parseTextRecord() as this is also used by the
  Smart Poster parser.
  */
QString NfcInfo::textRecordToString(const QNdefNfcTextRecord& textRecord)
{
    QString txt("Title: " + textRecord.text() + "\n");
    txt.append("Locale: " + textRecord.locale() + "\n");
    const QString textEncoding = textRecord.encoding() == QNdefNfcTextRecord::Utf8 ? "UTF-8" : "UTF-16";
    txt.append("Encoding: " + textEncoding + "\n");
    return txt;
}

/*!
  \brief Create a textual description of the contents of the
  Smart Poster (Sp) record.

  \param record the record to analyze
  \return plain text description of the record contents.
  */
QString NfcInfo::parseSpRecord(const NdefNfcSpRecord& record)
{
    QString tagContents("[Smart Poster]\n");

    // Uri
    tagContents.append("Uri: " + record.uri().toString() + "\n");

    // Title
    tagContents.append("Title count: " + QString::number(record.titleCount()) + "\n");
    if (record.titleCount() > 0)
    {
        foreach (QNdefNfcTextRecord curTitle, record.titles()) {
            tagContents.append(textRecordToString(curTitle));
        }
    }

    // Action
    if (record.actionInUse())
    {
        QString spActionString = "Unknown";
        switch (record.action())
        {
        case NdefNfcSpRecord::DoAction:
            spActionString = "Do Action";
            break;
        case NdefNfcSpRecord::SaveForLater:
            spActionString = "Save for later";
            break;
        case NdefNfcSpRecord::OpenForEditing:
            spActionString = "Open for editing";
            break;
        case NdefNfcSpRecord::RFU:
            spActionString = "RFU";
            break;
        }
        tagContents.append("Action: " + spActionString + "\n");
    }

    // Size
    if (record.sizeInUse())
    {
        tagContents.append("Size: " + QString::number(record.size()) + "\n");
    }

    // Type
    if (record.typeInUse())
    {
        tagContents.append("Type: " + record.type() + "\n");
    }

    // Image
    if (record.imageInUse())
    {
        NdefNfcMimeImageRecord spImageRecord = record.image();
        QByteArray imgFormat = spImageRecord.format();
        if (!imgFormat.isEmpty()) {
            tagContents.append("Image format: " + imgFormat + "\n");
        }
        QImage spImage = spImageRecord.image();
        if (!spImage.isNull())
        {
            if (imgCache) {
                const int imgId = imgCache->addImage(spImage);
                qDebug() << "Stored image into cache, id: " << imgId;
                emit nfcTagImage(imgId);
            } else {
                qDebug() << "Image cache not set";
            }
        }
    }

    // Raw contents
    //tagContents.append(record.rawContents());
    return tagContents;
}

/*!
  \brief Create a textual description of the contents of the
  image mime type record.

  The parsing works regardless of the actual image format used and
  supports all image formats available to Qt.

  \param record the record to analyze
  \return plain text description of the record contents.
  */
QString NfcInfo::parseImageRecord(const NdefNfcMimeImageRecord& record)
{
    QString tagContents("[Image]\n");
    // Read image format (png, gif, jpg, etc.)
    QByteArray imgFormat = record.format();
    if (!imgFormat.isEmpty()) {
        tagContents.append("Format: " + imgFormat + "\n");
    }
    // Retrieve the image
    QImage img = record.image();
    if (!img.isNull()) {
        // Image size
        const QSize imgSize = img.size();
        tagContents.append("Width: " + QString::number(imgSize.width()) + ", height: " + QString::number(imgSize.height()));

        // Store the image in the cache to show it on the screen
        if (imgCache) {
            const int imgId = imgCache->addImage(img);
            qDebug() << "Stored image into cache, id: " << imgId;
            emit nfcTagImage(imgId);
        } else {
            tagContents.append("Error: Image cache not set\n");
        }
    }

    return tagContents;
}

/*!
  \brief Create a textual description of the contents of the
  Uri record.

  \param record the record to analyze
  \return plain text description of the record contents.
  */
QString NfcInfo::parseVcardRecord(NdefNfcMimeVcardRecord& record)
{
    QString tagContents("[vCard]\n");

    // Parse the list of contacts from the record
    QList<QContact> contacts = record.contacts();
    if (!contacts.isEmpty())
    {
        // At least one contact could be successfully retrieved
        // (will in most cases only be one in the NFC scenario)
        foreach (QContact curContact, contacts) {
            //const QString contactDisplayLabel = curContact.displayLabel();
            // Get a list containing all details of the contact, for easier display.
            QList<QContactDetail> details = curContact.details();

            foreach (QContactDetail curDetail, details) {
                // Go through all contact details
                const QString detailName = curDetail.definitionName();
                if (detailName == QContactThumbnail::DefinitionName) {
                    // Special case: image
                    // Fetch the thumbnail and store it in the image cache.
                    QContactThumbnail contactThumb = curContact.detail<QContactThumbnail>();
                    QImage contactThumbImage = contactThumb.thumbnail();
                    if (!contactThumbImage.isNull())
                    {
                        if (imgCache) {
                            const int imgId = imgCache->addImage(contactThumbImage);
                            qDebug() << "Stored image into cache, id: " << imgId;
                            emit nfcTagImage(imgId);
                        } else {
                            qDebug() << "Image cache not set";
                        }
                    }
                } else {
                    // Any other detail except the image:
                    // add the detail name and its contents to the description.
                    tagContents.append(detailName + ": ");
                    // We just add all values related to the detail converted to a string.
                    QVariantMap valueMap = curDetail.variantValues();
                    foreach (QVariant curValue, valueMap) {

                        tagContents.append(curValue.toString() + " ");
                    }
                    tagContents.append("\n");
                }
            }
        }
    }
    else
    {
        // Error parsing the vCard into a QContact instance
        // Show the error message.s
        QString errorMsg = record.error();
        if (!errorMsg.isEmpty()) {
            tagContents.append(errorMsg);
        }
    }
    return tagContents;
}


/*!
  \brief Create the message for writing to the tag and attempt
  to write it.

  This method will be extended in future releases of this app.

  \return if it was already possible to write to the tag. If
  false is returned, the message is cached and will be written
  when a writable target is available. Only one message is cached;
  if this method is called a second time before the first message
  is actually written to a tag, the old message will be discarded
  and only the later one written to the tag.
  */
bool NfcInfo::nfcWriteTag(const QString &nfcTagText)
{
    // Create a new NDEF message
    QNdefMessage message;

    // Try to convert the String to a URL and check if it is
    // valid. This is only a rather simple conversion:
    // We additionally check if the URL contains a '.' character,
    // as otherwise a word like "hello" would be converted to
    // "http://hello". Obviously, this assumption doesn't work when
    // you want to store telephone numbers as URIs; but this example is
    // only intended for URLs and plain text.
    QUrl convertedUrl = QUrl::fromUserInput(nfcTagText);
    if (convertedUrl.isValid() && nfcTagText.contains('.'))
    {
        // The string was a URL, so create a URL record
        QNdefNfcUriRecord uriRecord;
        uriRecord.setUri(convertedUrl);
        message.append(uriRecord);
        qDebug() << "Creating URI message ...";
    } else {
        // Write a text record to the tag
        QNdefNfcTextRecord textRecord;
        textRecord.setText(nfcTagText);
        // Use the English locale.
        textRecord.setLocale("en");
        message.append(textRecord);
        qDebug() << "Creating Text message ...";
    }

    QByteArray rawMessage = message.toByteArray();
    emit nfcStatusUpdate("Created message - size: " + QString::number(rawMessage.size()) + " bytes");

    // Write the message (containing either a URL or plain text) to the target.
    cachedNdefMessage = message;
    pendingWriteNdef = true;
    return writeCachedNdefMessage();
}

/*!
  \brief Attempt to write the currently cached message to the tag.

  \return true if it was possible to send the request to the tag.
  */
bool NfcInfo::writeCachedNdefMessage()
{
    bool success = false;
    if (pendingWriteNdef)
    {
        if (cachedTarget)
        {
            // Check target access mode
            QNearFieldManager::TargetAccessModes accessModes = nfcManager->targetAccessModes();
            // Writing access is active - we should be able to write
            if (accessModes.testFlag(QNearFieldManager::NdefWriteTargetAccess))
            {
#ifdef MEEGO_EDITION_HARMATTAN
                cachedRequestId = cachedTarget->writeNdefMessages(QList<QNdefMessage>() << cachedNdefMessage);
                nfcManager->setTargetAccessModes(QNearFieldManager::NdefReadTargetAccess);
#else
                //cachedRequestId = cachedTarget->writeNdefMessages(QList<QNdefMessage>() << cachedNdefMessage);
                // QtM 1.2 beta doesn't return the request ID
                cachedTarget->writeNdefMessages(QList<QNdefMessage>() << cachedNdefMessage);
#endif
                pendingWriteNdef = false;
                emit nfcStatusUpdate("Writing message to the tag.");
                success = true;
            } else {
                // Device is not in writing mode
                emit nfcStatusUpdate("Please touch the tag again to write the message.");
            }
        } else {
            // Can't write - no cached target available
            emit nfcStatusUpdate("Please touch a tag to write the message.");
        }
    }
    return success;
}

/*!
  \brief Slot for handling when the target was lost (usually when
  it gets out of range.
  */
void NfcInfo::targetLost(QNearFieldTarget *target)
{
    cachedTarget = NULL;
    target->deleteLater();
    emit nfcStatusUpdate("Target lost");
}

/*!
  \brief Slot for handling an error with a request to the target.

  Emits the nfcTagError signal containing a description of the error.
  */
void NfcInfo::targetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &/*id*/)
{
    QString errorText("Error: " + convertTargetErrorToString(error));
    qDebug() << errorText;
    nfcTagError(errorText);
}

/*!
  \brief Slot called by Qt Mobility when a request has been completed.

  This method emits the nfcStatusUpdate signal to log the event in the
  user interface. In case the request was of a kind that contains a
  response, information about the response will also be emitted through
  another signal of the same type. In case the response is a byte array,
  it will be printed as a string of hex characters.
  */
void NfcInfo::requestCompleted(const QNearFieldTarget::RequestId &id)
{
    if (id == cachedRequestId) {
        emit nfcStatusUpdate("Active request completed.");
    } else {
        emit nfcStatusUpdate("Request completed.");
    }

    // Request the response
    if (cachedTarget)
    {
        QVariant response = cachedTarget->requestResponse(id);
        if (response.isValid()) {
            if (response.type() == QVariant::ByteArray) {
                emit nfcStatusUpdate("Response (" + QString(response.typeName()) + ")");
            } else {
                emit nfcStatusUpdate("Response (" + QString(response.typeName()) + "): " + response.toString());
            }
            if (response.type() == QVariant::ByteArray) {
                QByteArray p = response.toByteArray();
                QString arrayContents = "";
                for (int i = 0; i < p.size(); ++i) {
                    arrayContents.append(QString("0x") + QString::number(p.at(i), 16) + " ");
                }
                emit nfcStatusUpdate("\nRaw contents of payload:\n" + arrayContents + "\n");
            }
        }
    }
}

/*!
  \brief Slot called by Qt Mobility when an NDEF message was successfully
  written to a tag.

  Emits an nfcStatusUpdate signal to log this in the user interface.
  On MeeGo, both the requestCompleted() method and this method will be called
  when writing a tag.
  */
void NfcInfo::ndefMessageWritten()
{
    emit nfcStatusUpdate("Message written to the tag.");
}

/*!
  \brief Return a textual description of the \a error.
  */
QString NfcInfo::convertTargetErrorToString(QNearFieldTarget::Error error)
{
    QString errorString = "Unknown";
    switch (error)
    {
    case QNearFieldTarget::NoError:
        errorString = "No error has occurred.";
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
#ifdef MEEGO_EDITION_HARMATTAN
    case QNearFieldTarget::UnknownError:
        errorString = "Unknown error.";
        break;
#endif
    }
    return errorString;
}

/*!
  \brief Return a textual description of the NFC target \a type.
  */
QString NfcInfo::convertTagTypeToString(const QNearFieldTarget::Type type)
{
    QString tagType = "Unknown";
    switch (type)
    {
    case QNearFieldTarget::AnyTarget:
        tagType = "This value is only used when registering handlers to indicate that any compatible target can be used.";
        break;
    case QNearFieldTarget::ProprietaryTag:
        tagType = "An unidentified proprietary target tag.";
        break;
    case QNearFieldTarget::NfcTagType1:
        tagType = "An NFC tag type 1 target.";
        break;
    case QNearFieldTarget::NfcTagType2:
        tagType = "An NFC tag type 2 target.";
        break;
    case QNearFieldTarget::NfcTagType3:
        tagType = "An NFC tag type 3 target.";
        break;
    case QNearFieldTarget::NfcTagType4:
        tagType = "An NFC tag type 4 target.";
        break;
    case QNearFieldTarget::MifareTag:
        tagType = "A Mifare target.";
        break;
    case QNearFieldTarget::NfcForumDevice:
        tagType = "An NFC Forum device target.";
        break;
    }
    return tagType;
}

/*!
  \brief Return a textual description of the \a typeName of the NDEF record.
  */
QString NfcInfo::convertRecordTypeNameToString(const QNdefRecord::TypeNameFormat typeName)
{
    QString typeNameString = "Unknown";
    switch (typeName)
    {
    case QNdefRecord::Empty:
        typeNameString = "An empty NDEF record (does not contain a payload)";
        break;
    case QNdefRecord::NfcRtd:
        typeNameString = "NFC RTD Specification";
        break;
    case QNdefRecord::Mime:
        typeNameString = "RFC 2046 (Mime)";
        break;
    case QNdefRecord::Uri:
        typeNameString = "RFC 3986 (Url)";
        break;
    case QNdefRecord::ExternalRtd:
        typeNameString = "External type names (described the NFC RTD Specification)";
        break;
    case QNdefRecord::Unknown:
        typeNameString = "The type of the record is unknown and should be treated similar to content with MIME type 'application/octet-stream' without further context";
        break;
    }
    return typeNameString;
}


#ifdef Q_OS_SYMBIAN
/*!
  \brief Slot will be executed by the nfcsettings component whenever
  the NFC availability changes in the phone.

  This can also happen at runtime, for example when the user switches
  on NFC through the settings app. Emits an nfcStatusError or nfcStatusUpdate
  signal with the new information.
  */
void NfcInfo::handleNfcModeChange(NfcSettings::NfcMode nfcMode)
{
    switch (nfcMode) {
    case NfcSettings::NfcModeNotSupported:
        // NFC is not currently supported. It is not possible to distinguish
        // whether a firmware update could enable NFC features based solely
        // on the value of the nfcMode parameter. The return value of
        // NfcSettings::nfcFeature() indicates whether a firmware update is
        // applicable to this device.
        emit nfcStatusError(tr("NFC is not currently supported."));
        break;
    case NfcSettings::NfcModeUnknown:
        // NFC is supported, but the current mode is unknown at this time.
        emit nfcStatusError(tr("NFC is supported, but the current mode is unknown at this time."));
        break;
    case NfcSettings::NfcModeOn:
        // NFC is supported and switched on.
        emit nfcStatusUpdate(tr("NFC is supported and switched on."));
        break;
    case NfcSettings::NfcModeCardOnly:
        // NFC hardware is available and currently in card emulation mode.
        emit nfcStatusError(tr("NFC hardware is available and currently in card emulation mode."));
        break;
    case NfcSettings::NfcModeOff:
        // NFC hardware is available but currently switched off.
        emit nfcStatusError(tr("NFC hardware is available but currently switched off."));
        break;
    default:
        break;
    }
}

/*!
  \brief Handle any error that might have occured when checking
  the NFC support on the phone.
  */
void NfcInfo::handleNfcError(NfcSettings::NfcError nfcError, int error)
{
    // The platform specific error code is ignored here.
    Q_UNUSED(error)

    switch (nfcError) {
    case NfcSettings::NfcErrorFeatureSupportQuery:
        // Unable to query NFC feature support.
        emit nfcStatusError(tr("Unable to query NFC feature support."));
        break;
    case NfcSettings::NfcErrorSoftwareVersionQuery:
        // Unable to query device software version.
        emit nfcStatusError(tr("Unable to query device software version."));
        break;
    case NfcSettings::NfcErrorModeChangeNotificationRequest:
        // Unable to request NFC mode change notifications.
        emit nfcStatusError(tr("Unable to request NFC mode change notifications."));
        break;
    case NfcSettings::NfcErrorModeChangeNotification:
        // NFC mode change notification was received, but caused an error.
        emit nfcStatusError(tr("NFC mode change notification was received, but caused an error."));
        break;
    case NfcSettings::NfcErrorModeRetrieval:
        // Unable to retrieve current NFC mode.
        emit nfcStatusError(tr("Unable to retrieve current NFC mode."));
        break;
    default:
        break;
    }
}


#endif
