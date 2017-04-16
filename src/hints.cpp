/*
 * hints.cpp
 *
 *  Created on: Aug 1, 2015
 *      Author: jasp
 */

#include "applicationui.hpp"
#include "ViBrowserTab.hpp"

void ApplicationUI::SL_updateHintFilter(QString newFilter)
{

    newFilter = newFilter.toLower();
    qDebug() << "updating filter: " << newFilter;
    this->hintFilter = newFilter;
    currentTab->tabwebview->postMessage(newFilter);

    QChar lastChar = newFilter.right(1)[0];
    if (lastChar.isDigit()) {
        keyStack.prepend(lastChar.toAscii());
        checkHints(hintLines, keyStack, currentTab->tabwebview);
    }
}


void ApplicationUI::stopHintMode(WebView *webview) {
    this->hintFilter= "";
    webview->evaluateJavaScript("exitHintMode()", JavaScriptWorld::Normal);
    currentTab->tabwebview->requestFocus();
}

void ApplicationUI::SL_selectHint(bb::cascades::AbstractTextControl* control) {
    QString hintUrl = hintLines[0].split(" ").at(0);
    qDebug() << "On selectHint, hitting first link";
    stopHintMode(currentTab->tabwebview);
    currentTab->tabwebview->setUrl(QUrl::fromEncoded(hintUrl.toUtf8()).toString());
}

void ApplicationUI::checkHints(QStringList hintLines, QList<char> keyStack, WebView *webview) {

    //TODO: Don't parse keystack, just get it from filter

    //only one hint on screen
    qDebug() << "hintLines length: " << hintLines.length();
    if (hintLines.length() == 1) {
        QString hintUrl = hintLines[0].split(" ").at(0);
        qDebug() << "Last hint url is " << hintUrl;
        stopHintMode(webview);
        qDebug() << "going to " << QUrl::fromEncoded(hintUrl.toUtf8()).toString();
        webview->setUrl(QUrl::fromEncoded(hintUrl.toUtf8()).toString());
    }

    //Get most recent numbers typed from key history
    int i=0;
    QString numRequest = "";
    while (QChar(keyStack.at(i)).isNumber()) {
        numRequest.prepend(keyStack.at(i));
        i++;
    }

    //get length of biggest hint (eg, if there are 10 hints, this should be 2)
    int numHintsLength = 0;
    for (int i=0;i<hintLines.length();i++) {
        QString line = hintLines.at(i);
        QString hintnum = line.split(" ").at(1);
        if (hintnum.length() > numHintsLength)
            numHintsLength = hintnum.length();
    }

    qDebug () << "Number requested is " << numRequest << ",need to beat len " <<numHintsLength;

    if (numHintsLength <= numRequest.length()) {
        for (int i=0;i<hintLines.length();i++) {
            QStringList line = hintLines.at(i).split(" ");
            if (line.at(1) == numRequest) {
                QString hintUrl = line.at(0);
                stopHintMode(webview);
                webview->setUrl(QUrl::fromEncoded(hintUrl.toUtf8()).toString());
            }
        }
        stopHintMode(currentTab->tabwebview);
    }
}

void ApplicationUI::startHintMode(WebView *webview) {
    currentTab->tabhintbar->setText("");
    webview->postMessage("");
    currentTab->tabhintbar->requestFocus();
}

void ApplicationUI::SL_stopHintMode(bool on) {
    if (!on)
        stopHintMode(currentTab->tabwebview);
}

void ApplicationUI::SL_receiveHintsFromJS(QVariantMap message) {
    qDebug() << "in receive void initHintJS()";

    //No hints
    qDebug() << "raw data received: " << message["data"];

    if (message["data"].toString().trimmed().size() == 0)
            return;

    this->hintLines = message["data"].toString().split("\n", QString::SkipEmptyParts);

    for(int i=0;i<hintLines.length();i++) {
        qDebug() << "Line"<<(i+1)<<":" << hintLines.at(i);
    }

    checkHints(this->hintLines, this->keyStack, currentTab->tabwebview);
}


void ApplicationUI::initHintJS() {

    QFile hintjsfile("app/native/assets/hints.js");
    hintjsfile.open(QIODevice::ReadOnly | QIODevice::Text);

    while (!hintjsfile.atEnd()) {
        QByteArray line = hintjsfile.readLine();
        hintJS += QString(line);
    }

    qDebug() << "HintJS content size: " << hintJS.size();
}
