/*
 * navigation.cpp
 *
 *  Created on: Aug 1, 2015
 *      Author: jasp
 */

#include "applicationui.hpp"
#include "ViBrowserTab.hpp"

void ApplicationUI::goPathUp(WebView *webview) {

    QString url = QUrl(currentTab->tabtitlebar->text()).toString();//webview->url().path();

    url.truncate(url.lastIndexOf("/", -2));

    qDebug() << "path after" << url;

    QUrl newUrl(url);
    webview->setUrl(newUrl);
}


void ApplicationUI::tabNew(QString urlBarContent, Page *root) {
    this->tabIndex++;
    ViBrowserTab *newTab = new ViBrowserTab(this);
    this->currentTab = newTab;
    this->tabStack.insert(tabIndex, currentTab);
    //The cast to control was not necessary when this was inline...
    root->setContent((Control*)newTab->tabcontainer);
    updateTabSelection(currentTab->tabselectcounter, tabIndex, tabStack.count());

    currentTab->taburlbar->requestFocus();
    currentTab->taburlbar->setText(urlBarContent);

}

void ApplicationUI::switchTab(Page *root, int offset)
{
    if (tabIndex + offset == tabStack.count() || tabIndex + offset < 0)
        return;

    this->tabIndex += offset;

    this->currentTab = tabStack.at(tabIndex);
    root->setContent((Control*)currentTab->tabcontainer);
    updateTabSelection(currentTab->tabselectcounter, tabIndex, tabStack.count());

    currentTab->tabwebview->requestFocus();
}

void ApplicationUI::SL_updateUrl(QUrl newUrl) {
    qDebug() << "Updated titlebar";
    currentTab->tabtitlebar->setText(newUrl.toString());
}

void ApplicationUI::SL_goURL(bb::cascades::AbstractTextControl* control) {
    QUrl newUrl = QUrl().fromUserInput(control->text());
    currentTab->tabwebview->setUrl(newUrl);
}

void ApplicationUI::goPageBack(WebView *webview) {
    webview->goBack();
}

void ApplicationUI::goPageForward(WebView *webview) {
    webview->goForward();
}

void ApplicationUI::tabDel(int move) {

    if (tabStack.count() <= 1)
            return;

    //emulates vimperator's behavior:
    //At first and last tab, x/X do the same thing
    if (tabIndex+move < 0)
        move = 0;

    if (tabIndex >= tabStack.count()-1) {
        move = -1;
    }

    delete this->tabStack.at(tabIndex);
    this->tabStack.removeAt(tabIndex);
    tabIndex += move;
    this->currentTab = tabStack.at(tabIndex);
    root->setContent((Control*)currentTab->tabcontainer);
    updateTabSelection(currentTab->tabselectcounter, tabIndex, tabStack.count());

}


void ApplicationUI::updateTabSelection(Label *tabcounter, int tabindex, int numtabs) {
    QString index = QString().setNum(tabindex+1);
    QString tabcount = QString().setNum(numtabs);
    tabcounter->setText( index + "/" + tabcount);
    currentTab->tabtitlebar->setText(currentTab->tabwebview->url().toString());
}
