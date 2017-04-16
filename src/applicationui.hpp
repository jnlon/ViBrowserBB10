/*
 * Copyright (c) 2011-2015 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_

#include <QObject>
#include <QUrl>
#include <QList>
#include <QHash>
#include <QVariantMap>

#include <bb/cascades/KeyEvent>
#include <bb/cascades/KeyListener>
#include <bb/cascades/controls/webview.h>
#include <bb/cascades/controls/scrollview.h>
#include <bb/cascades/controls/page.h>
#include <bb/cascades/controls/textfield.h>
#include <bb/cascades/Label>
#include <bb/cascades/WebFindFlag>
#include <bb/cascades/AbstractTextControl>
#include <bb/cascades/WebLoadStatus>
#include <bb/cascades/WebLoadRequest>
#include <bb/cascades/WebFindFlag>

#include <bb/system/Clipboard.hpp>
#include <bb/system/SystemToast.hpp>

#include <bb/data/SqlDataAccess>



namespace bb
{
    namespace cascades
    {
        class LocaleHandler;
    }
}

//Forward Declarations
class QTranslator;
class ViBrowserTab;

using namespace bb::cascades;

/*!
 * @brief Application UI object
 *
 * Use this object to create and init app UI, to create context objects, to register the new meta types etc.
 */

struct URLRecord {
    QString url;
    QString title;
    QString domain;
};


class ApplicationUI : public QObject
{
    Q_OBJECT
public:
    ApplicationUI();
    virtual ~ApplicationUI() {};

    ViBrowserTab *currentTab;
    Page *root;
    QList<char> keyStack;
    QList<ViBrowserTab*> tabStack;
    QList<URLRecord> history;
    QList<URLRecord> bookmarks;
    bb::data::SqlDataAccess *db;

    QHash<QString, void(*)(ApplicationUI*)> keyBindings;
    bool ignorekeys;
    QString hintFilter;
    QString hintJS;
    QString searchString;
    QStringList hintLines;
    int tabIndex;
    QList<QString> sortedKeyBinds;

    //Config
     struct Config {
        float PixelRatioModifier;
        bool JavascriptEnabled;
        bool ImagesEnabled;
        bool CookiesEnabled;
        bool DesktopMode;
        QString SearchEngineString;
        QString HomePage;
    } config;

    bb::system::SystemToast *toaster;
    bb::system::Clipboard *clipboard;

    void scrollRight(float percent, ScrollView *scrollview);
    void scrollDown(float percent, ScrollView *scrollview);
    void tabDel(int move);
    void switchTab(Page *page, int offset);
    void tabNew(QString urlBarContent, Page *page);
    void copyURL(QUrl currentURL, bb::system::Clipboard *clipboard);
    void zoomIn(float percent, ScrollView *scrollview);
    void zoomReset(ScrollView *scrollview);
    void goClipURL(WebView *webview, bb::system::Clipboard *clip);
    void goPageForward(WebView *webview);
    void goPageBack(WebView *webview);
    void goPathUp(WebView *webview);
    void pageRefresh(WebView *webview);
    void searchText(QString searchString, WebView *webview, WebFindFlag::Types flags);

    void checkHints(QStringList hintLines, QList<char> keyStack, WebView *webview);
    void updateTabSelection(Label *tabselectcounter, int tabindex, int numtabs);
    void exitApp();
    void viewSource(WebView *webview);


    void openBar(TextField* somebar, QString content);

    //hints
    void startHintMode(WebView *webview);
    void stopHintMode(WebView *webview);


    //misc
    void initKeyBindings();
    void initSortedKeyBinds();
    void initHintJS();
    void initConfig();
    void initSQLDB();
    void updateTabCounter();
    void disconnectInputBar();
    TextField* getVisibleBar();
    void setCommandConfig(QString option);
    void printSettingsToHtml();
    void printUrlRecordToHtml(QList<URLRecord>);
    void configParser();
    void configWriter();
    void updateConfig();
    void remapKey(QString ls, QString rs);
    void clearHistory();
    void showSqlAsPage(QVariant qvblob, QString title);
    void addHistory();
    void addBookmark();

private slots:
    void SL_onKeyPressedHandler(bb::cascades::KeyEvent* keyevent);
    void SL_onKeyLongPressedHandler(bb::cascades::KeyEvent* keyevent);

    //for bars
    void SL_selectHint(bb::cascades::AbstractTextControl* control);
    void SL_goURL(bb::cascades::AbstractTextControl* control);
    void SL_runCommand(bb::cascades::AbstractTextControl* control);
    void SL_searchPageText(bb::cascades::AbstractTextControl* control);
    void SL_searchEngineQuery(bb::cascades::AbstractTextControl* control);
    void SL_onBarKeyPressed(bb::cascades::KeyEvent* keyevent);


    //void SL_toggleInputBar(bool focused);

    void SL_updateUrl(QUrl newUrl);
    void SL_receiveHintsFromJS(QVariantMap message);
    void SL_menuAbout();
    void SL_menuHelp();
    void SL_pageLoad(bb::cascades::WebLoadRequest* request);
    void SL_updateHintFilter(QString newFilter);
    void SL_stopHintMode(bool on);

};

//void sortBindsByLength(QList<QString> *keys);

#endif /* ApplicationUI_HPP_ */
