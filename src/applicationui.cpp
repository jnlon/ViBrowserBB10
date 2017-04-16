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


#include "ViBrowserTab.hpp"

#include <QDebug>
#include <QUrl>
#include <QList>
#include <QFile>
#include <QHash>

#include <bb/cascades/Application>
#include <bb/cascades/AbstractTextControl>
#include <bb/cascades/TextField>
#include <bb/cascades/Page>
#include <bb/cascades/Container>
#include <bb/cascades/Menu>
#include <bb/cascades/ActionItem>
#include <bb/cascades/HelpActionItem>

#include <bb/cascades/WebView>
#include <bb/cascades/WebFindFlag>
#include <bb/cascades/WebLoadStatus>
#include <bb/cascades/WebSettings>
#include <bb/cascades/ScrollAnimation>
#include <bb/cascades/ScrollView>
#include <bb/cascades/KeyEvent>
#include <bb/cascades/KeyListener>

#include <bb/system/SystemToast>
#include <bb/system/Clipboard>
#include <bps/virtualkeyboard.h>
#include <bb/data/SqlDataAccess.hpp>

#define ALT ((char)-23)
#define LSHIFT ((char)-31)
#define RSHIFT ((char)-30)
#define BACKSPACE ((char)8)
#define ENTER ((char)13)

/*TODO:
 *
 * To complete before sumbission:
 * -Rudimentary command mode
 *   -bmarks
 *   -history
 *   -!clear bmarks
 *   -!clear history
 *
 * -Docs (Intro/Home, FAQ, key reference, command reference, about)
 * -View Page Source
 * -Test with virtualkeyboard || Enable gesture navigation
 * -Search engine (s)
 * -History/Bookmarks (just an html page)
 * -URL blocker
 *
 * ----
 *
 * Stretch features:
 * -Macros
 * -Split window
 * -Page marks
 * -Reader mode
 * -Private Browsing Mode
 *
 * Other:
 * -Fix up use of globals in below functions (no currentTab!)
 * -Add a reload/page loading indicator
 * -Side Scrollable titlebar
 * -Get open in new tab context menu working for links
 * -"F", hints in new tabs (tricky!)
 * -Get space working with hints, also, why is bar borking?
 * -Turn printhtml into navication to temporary vibrowserrc(.tmp)
 *  -Then, on save, just rename.
 * -Reorg:
 *   Move Keybinding stuff into their own file
 *
 * Idea: have global last nums? Seriously, need to unmangle hints still
 *
 *
 */

using namespace bb::cascades;

 void K_scrollUp(ApplicationUI *appui)      { appui->scrollDown(-5.0f, appui->currentTab->tabscrollview); }
 void K_scrollDown(ApplicationUI *appui)    { appui->scrollDown(5.0f, appui->currentTab->tabscrollview); }
 void K_scrollRight(ApplicationUI *appui)   { appui->scrollRight(5.0f, appui->currentTab->tabscrollview); }
 void K_scrollLeft(ApplicationUI *appui)    { appui->scrollRight(-5.0f, appui->currentTab->tabscrollview);  }
 void K_scrollTop(ApplicationUI *appui)     { appui->scrollDown(-999999.0f, appui->currentTab->tabscrollview);}
 void K_scrollBottom(ApplicationUI *appui)  { appui->scrollDown(999999.0f, appui->currentTab->tabscrollview);}
 void K_scrollPageUp(ApplicationUI *appui)  { appui->scrollDown(-100.0f, appui->currentTab->tabscrollview);}
 void K_scrollPageDown(ApplicationUI *appui) { appui->scrollDown(100.0f, appui->currentTab->tabscrollview);}
 void K_scrollHalfPageUp(ApplicationUI *appui) { appui->scrollDown(-50.0f, appui->currentTab->tabscrollview); }
 void K_scrollHalfPageDown(ApplicationUI *appui) { appui->scrollDown(50.0f, appui->currentTab->tabscrollview); }
 void K_tabNew(ApplicationUI *appui)             { appui->tabNew(appui->currentTab->tabwebview->url().toEncoded(),appui->root);}
 void K_tabNewBlank(ApplicationUI *appui)        { appui->tabNew("", appui->root);}
 void K_tabNext(ApplicationUI *appui)       { appui->switchTab(appui->root, 1);}//{ appui->tabNext(appui->root);}
 void K_tabPrev(ApplicationUI *appui)       { appui->switchTab(appui->root, -1);}//{ appui->tabPrev(appui->root);}
 void K_tabDelNext(ApplicationUI *appui) { appui->tabDel(0);}
 void K_tabDelPrev(ApplicationUI *appui) { appui->tabDel(-1);}
 void K_zoomIn(ApplicationUI *appui)        { appui->zoomIn(5, appui->currentTab->tabscrollview);}
 void K_zoomOut(ApplicationUI *appui)       { appui->zoomIn(-5, appui->currentTab->tabscrollview);}
 void K_zoomReset(ApplicationUI *appui)     { appui->zoomReset(appui->currentTab->tabscrollview);}
 void K_yankURL(ApplicationUI *appui)       { appui->copyURL(appui->currentTab->tabwebview->url(), appui->clipboard);}
 void K_pasteURLNewTab(ApplicationUI *appui){ appui->tabNew(appui->clipboard->value("text/plain"), appui->root);}
 void K_startHintMode(ApplicationUI *appui) { appui->startHintMode(appui->currentTab->tabwebview);}
 void K_startHintModeNewTab(ApplicationUI *appui) { appui->startHintMode(appui->currentTab->tabwebview);}

 void K_goPageBack(ApplicationUI *appui)    { appui->goPageBack(appui->currentTab->tabwebview);}
 void K_goPageForward(ApplicationUI *appui) { appui->goPageForward(appui->currentTab->tabwebview);}
 void K_goPathUp(ApplicationUI *appui)      { appui->goPathUp(appui->currentTab->tabwebview);}

 void K_searchTextNext(ApplicationUI *appui)    { appui->searchText(appui->searchString,appui->currentTab->tabwebview, 0);}
 void K_searchTextPrev(ApplicationUI *appui)    { appui->searchText(appui->searchString,appui->currentTab->tabwebview, WebFindFlag::Backward);}
 void K_pageRefresh(ApplicationUI *appui)    { appui->pageRefresh(appui->currentTab->tabwebview);}

 void K_startSearchTextNext(ApplicationUI *appui)    { appui->searchString = ""; appui->openBar(appui->currentTab->tabtextsearchbar, "");}
 void K_startSearchTextPrev(ApplicationUI *appui)    { appui->searchString = ""; appui->openBar(appui->currentTab->tabtextsearchbar, "");} //appui->openTextSearchBar
 void K_openUrlBar(ApplicationUI *appui)    { appui->openBar(appui->currentTab->taburlbar, appui->currentTab->tabwebview->url().toString());}
 void K_openUrlBarBlank(ApplicationUI *appui) { appui->openBar(appui->currentTab->taburlbar, "");} //openUrlBar
 void K_commandMode(ApplicationUI *appui)    { appui->openBar(appui->currentTab->tabcommandbar, ""); }   //appui->openCommandBar(); }
 void K_openWebSearchBar(ApplicationUI *appui)    {  appui->openBar(appui->currentTab->tabwebsearchbar, "");}   //appui->openWebSearchBar(); }
 void K_pasteURL(ApplicationUI *appui)      { appui->openBar(appui->currentTab->taburlbar, appui->clipboard->value("text/plain"));}

 void K_goHomePage(ApplicationUI *appui)    { appui->currentTab->tabwebview->setUrl(QUrl("/"));}
 void K_showKeyboard(ApplicationUI *appui)    { virtualkeyboard_show();} //Test in VirtualMachine on return!
 void K_viewSource(ApplicationUI *appui)    { appui->viewSource(appui->currentTab->tabwebview); }
 void K_addBookmark(ApplicationUI *appui)    {appui->addBookmark(); }
 void K_exit(ApplicationUI *appui)          { appui->exitApp();}

ApplicationUI::ApplicationUI():
        QObject()
{

    //Init the menu
    ActionItem *about = ActionItem::create()
        .title("About")
        .image("asset:///ic_feedback.png");

    HelpActionItem *help = HelpActionItem::create();

    Menu *menu = Menu::create()
        .addAction(about)
        .help(help);

    QObject::connect(about, SIGNAL(triggered()), this, SLOT(SL_menuAbout()));
    QObject::connect(help, SIGNAL(triggered()), this, SLOT(SL_menuHelp()));

    Application::instance()->setMenu(menu);

    //Init the browser
    initKeyBindings();
    initHintJS();
    initConfig();
    initSQLDB();

    this->searchString = "";
    this->ignorekeys = false;

    this->tabIndex = 0;
    this->root = new Page;

    this->currentTab = new ViBrowserTab(this);
    this->clipboard = new bb::system::Clipboard;
    this->toaster = new bb::system::SystemToast;

    root->setContent(currentTab->tabcontainer);
    this->tabStack.push_back(currentTab);
    updateTabSelection(currentTab->tabselectcounter, tabIndex, tabStack.count());
    Application::instance()->setScene(root);
}


void ApplicationUI::SL_menuAbout() {
    qDebug() << "about!";
    currentTab->tabwebview->setUrl(QUrl("app/native/assets/hints.js"));
}

void ApplicationUI::SL_menuHelp() {
    qDebug() << "help!";
    currentTab->tabwebview->setUrl(QUrl("app/native/assets/hints.js"));
}

void ApplicationUI::SL_onKeyLongPressedHandler(bb::cascades::KeyEvent* keyevent) {
    char key = (char)keyevent->key();

    qDebug() << "Key long pressed: " << key;

    if ( key == 'I' && !ignorekeys) {
        toaster->setBody("Ignore keys on!");
        toaster->show();
        this->ignorekeys = true;
    }
    else if ( key == 'I' && ignorekeys) {
        toaster->setBody("Ignore keys off!");
        toaster->show();
        this->ignorekeys = false;
    }
}

void ApplicationUI::SL_onKeyPressedHandler(bb::cascades::KeyEvent* keyevent)
{

    if (ignorekeys)
        return;

    if (keyStack.size() > 5)
        keyStack.pop_back();

    char key = (char) keyevent->key();

    if (key == ALT || key == RSHIFT || key == LSHIFT)
        return;

    keyStack.push_front(key);

    qDebug() << "KeyStack is";
    for (int i = 0; i < keyStack.size(); i++)
        qDebug() << keyStack[i] << " (" << (int) keyStack[i] << ")";

    for (int i = 0; i < sortedKeyBinds.size(); i++) {

        int bindSize = sortedKeyBinds.at(i).length();

        if (bindSize > keyStack.length())
            continue;

        QString lastKeysPressed = "";
        for (int j = bindSize - 1; j >= 0; j--) {
            lastKeysPressed += keyStack[j];
        }

        if (sortedKeyBinds.at(i).startsWith(lastKeysPressed)) {
            //call assigned function pointer
            keyBindings[sortedKeyBinds[i]](this);
            return;
        }
    }
}

void ApplicationUI::openBar(TextField *bar, QString content) {
    bar->setText(content);
    bar->requestFocus();
}

void ApplicationUI::copyURL(QUrl currentURL, bb::system::Clipboard *clipboard) {

    qDebug() << "Copying to clipboard: " << currentURL.toEncoded();

    clipboard->clear();
    clipboard->insert("text/plain", currentURL.toEncoded());

    toaster->setBody("URL copied to Clipboard");
    toaster->show();

}

void ApplicationUI::exitApp() {
    Application::quit();
}


void ApplicationUI::searchText(QString searchString, WebView *webview, WebFindFlag::Types direction) {
    webview->findText(searchString, (WebFindFlag::Types)( 0x08 |  0x04 | (int)direction));
}

void ApplicationUI::viewSource(WebView *webview) {
    //TODO: See WebResourceRequestFilter
    //Will also allow adblocking
}


void ApplicationUI::addHistory() {

    URLRecord entry;
    entry.title = currentTab->tabwebview->title();
    entry.url = currentTab->tabwebview->url().toString();
    entry.domain = currentTab->tabwebview->url().host();

    if (entry.url.isEmpty() || entry.url.startsWith("about:"))
        return;

    if (entry.title.isEmpty())
        entry.title = "<no title>";

    if (entry.domain.isEmpty() && entry.url.contains("file:///"))
        entry.domain = "localhost";

    QString insertCommand = "INSERT INTO history (title, url, domain) VALUES ('" + entry.title + "', '" + entry.url + "' ,'" + entry.domain + "');";
    qDebug() << "insertcommand: " << insertCommand;
    db->execute(insertCommand);

    QVariant list = db->execute("SELECT * FROM history;");
    QVariantList qvlist = list.value<QVariantList>();

    for (int i=0;i<qvlist.size(); i++)
        qDebug() << qvlist.at(i);

}

void ApplicationUI::SL_searchPageText(bb::cascades::AbstractTextControl* control) {

    searchString = control->text();
    qDebug() << "finding" << searchString;

    //ORing the types directly seems to not work...
    //Backward = 0x01 , CaseSensitive = 0x02
    //WrapAroundDocument = 0x04 , HighlightAllOccurrences = 0x08
    currentTab->tabwebview->findText(searchString, (WebFindFlag::Types)( 0x08 |  0x04));

}

void ApplicationUI::SL_searchEngineQuery(bb::cascades::AbstractTextControl* control) {
    QString query = control->text();
    qDebug() << "Query is " << query;
    QString url = config.SearchEngineString.replace("%s", query);
    currentTab->tabwebview->setUrl(url);
}

void ApplicationUI::SL_runCommand(bb::cascades::AbstractTextControl* control) {

    QString command = control->text();

    if (command.isEmpty())
        return;

    QStringList tokens = command.split(" ");

    if (tokens[0] == "set" && tokens.length() == 2) {
        setCommandConfig(tokens[1]);
    }
    else if (tokens[0] == "set") {
        printSettingsToHtml();
        return;
    }

    if (command[0] == '!') {
            currentTab->tabwebview->evaluateJavaScript(command.remove(0,1) , JavaScriptWorld::Normal);
    }

    if (tokens[0] == "mkvibrc") {
        configWriter();
        currentTab->tabwebview->setUrl(QUrl( QString("file:///").append(QDir::currentPath())));
        configParser();
    }

    if (tokens[0] == "map" && tokens.length() == 3) {
        qDebug() << "mapping " << tokens[1] << " to " << tokens[2];
        remapKey(tokens[1], tokens[2]);
    }

    if (tokens[0] == "history") {
        showSqlAsPage(db->execute("SELECT * FROM history"), "History");
    }

    if (tokens[0] == "clearhistory") {
        clearHistory();
    }
}

void ApplicationUI::showSqlAsPage(QVariant blob, QString pagetitle) {
    QString page = "<html> <head> <title>" + pagetitle + "</title></head><body>" ;
    page += "<h3> " + pagetitle + "</h3>";

    QList<QVariant> qlist = blob.toList();

    for (int i=0;i<qlist.size();i++) {
        QMap<QString, QVariant> map = qlist.at(i).toMap();
        QString title = map["title"].toString();
        QString url = map["url"].toString();
        QString domain = map["domain"].toString();
        page += domain + ": " + "<a href='"+ url +"' >" + title + "</a><br>";
    }

    page += "</body></html>";

    currentTab->tabwebview->setHtml(page, QUrl("about:history"));
}

void ApplicationUI::addBookmark()
{
    //TODO: Make it so they can be removed

    URLRecord entry;
    entry.title = currentTab->tabwebview->title();
    entry.url = currentTab->tabwebview->url().toString();
    entry.domain = currentTab->tabwebview->url().host();

    if (entry.url.isEmpty() || entry.url.startsWith("about:"))
        return;

    if (entry.title.isEmpty())
        entry.title = "<no title>";

    if (entry.domain.isEmpty() && entry.url.contains("file:///"))
        entry.domain = "localhost";

    qDebug() << "Begin insert!";
    QString insertCommand = "INSERT INTO bookmark (title, url, domain) VALUES ('" + entry.title
            + "', '" + entry.url + "' ,'" + entry.domain + "');";
    qDebug() << "insertcommand: " << insertCommand;
    db->execute(insertCommand);

    qDebug() << "Begin query!";
    QVariant list = db->execute("SELECT * FROM bookmark;");
    QVariantList qvlist = list.value<QVariantList>();
    qDebug() << "begin!";
    for (int i = 0; i < qvlist.size(); i++) {
        qDebug() << qvlist.at(i);
    }
}

void ApplicationUI::remapKey(QString ls, QString rs) {

    if (keyBindings[rs] == NULL)
        return;

    this->keyBindings[ls] = this->keyBindings[rs];//function;
    initSortedKeyBinds();

}

void ApplicationUI::setCommandConfig(QString option) {

    if (option.contains("=")) {
        QStringList tokens = option.split("=");
        QString setting = tokens[0];
        QString value = tokens[1];

        if (setting == "searchenginestring")
                config.SearchEngineString = value;
        else if (setting == "pixelratiomodifier")
            config.PixelRatioModifier = value.toFloat();
        else if (setting == "homepage")
            config.HomePage = value;
    }
    else {
        bool optionSetting = true;
        if (option.startsWith("no")) {
            optionSetting = false;
            option = option.remove(0,2);
        }

        if (option == "image") {
            config.ImagesEnabled = optionSetting;
            qDebug() << "images is now: " << optionSetting;
        }
        else if (option == "js") {
            config.JavascriptEnabled = optionSetting;
        }
        else if (option == "cookies") {
            config.CookiesEnabled = optionSetting;
        }
        else if (option == "desktop") {
            config.DesktopMode = optionSetting;
        }
    }

    updateConfig();
}

void ApplicationUI::updateConfig() {
    WebView *w = currentTab->tabwebview;
    WebSettings *s = w->settings();

    s->setDevicePixelRatio(s->devicePixelRatio() * config.PixelRatioModifier);
    s->setTextAutosizingEnabled(!config.DesktopMode);
    s->setCookiesEnabled(config.CookiesEnabled);
    s->setImageDownloadingEnabled(config.ImagesEnabled);
    s->setJavaScriptEnabled(config.JavascriptEnabled);

    w->reload();

}

void ApplicationUI::SL_pageLoad(bb::cascades::WebLoadRequest *request)
{
    if (request->status() == WebLoadStatus::Succeeded) {
        currentTab->tabwebview->evaluateJavaScript(hintJS, JavaScriptWorld::Normal);
        addHistory();
        qDebug() << "Page Load Finished.";
    }
}

void ApplicationUI::SL_onBarKeyPressed(bb::cascades::KeyEvent *keyevent) {

    TextField *openBar = getVisibleBar();
    qDebug () << "pressed from bar " << (char)keyevent->key();
    qDebug () << "visible bar has " << openBar->text();

    if (openBar->text().isEmpty() &&
            (char)keyevent->key() == BACKSPACE) {
        stopHintMode(currentTab->tabwebview);
        currentTab->tabwebview->requestFocus();
    }

}

TextField* ApplicationUI::getVisibleBar() {
    qDebug() << "Made it to visible bar!";

    for (int i=0;i<5;i++) {
        if (currentTab->tabinputbars[i]->isVisible())
            return currentTab->tabinputbars[i];
    }
    return currentTab->tabinputbars[0]; //should not happen
}


void ApplicationUI::pageRefresh(WebView *webview) {
    webview->reload();
}

void ApplicationUI::initKeyBindings() {

    /* keyBindings is a QHash of (non class) function pointers.
     * This allows us to easily rebind keys, make macros, and execute
     * commands with more than one character
     *
     * When these functions are called, they are passed the current (and only)
     * instance of ApplicationUI, which gives access to all of the browser's resources.
     * These resources are then cherry picked and sent to functions that perform
     * the action described here.
     */

    keyBindings["h"] = K_scrollLeft;
    keyBindings["j"] = K_scrollDown;
    keyBindings["k"] = K_scrollUp;
    keyBindings["l"] = K_scrollRight;
    keyBindings["H"] = K_goPageBack;
    keyBindings["K"] = K_tabNext;
    keyBindings["J"] = K_tabPrev;
    keyBindings["L"] = K_goPageForward;
    keyBindings["u"] = K_scrollHalfPageUp;
    keyBindings["d"] = K_scrollHalfPageDown;
    keyBindings["b"] = K_scrollPageUp;
    keyBindings[" "] = K_scrollPageDown;
    keyBindings["o"] = K_openUrlBarBlank;
    keyBindings["O"] = K_openUrlBar;
    keyBindings["p"] = K_pasteURL;
    keyBindings["P"] = K_pasteURLNewTab;
    keyBindings["y"] = K_yankURL;
    keyBindings["t"] = K_tabNewBlank;
    keyBindings["T"] = K_tabNew;
    keyBindings["x"] = K_tabDelPrev;
    keyBindings["s"] = K_openWebSearchBar;
    keyBindings["X"] = K_tabDelNext;
    keyBindings["f"] = K_startHintMode;
    keyBindings["F"] = K_startHintModeNewTab;
    keyBindings["/"] = K_startSearchTextNext;
    keyBindings["?"] = K_startSearchTextPrev;
    keyBindings["n"] = K_searchTextNext;
    keyBindings["N"] = K_searchTextPrev;
    keyBindings["G"] = K_scrollBottom;
    keyBindings["+"] = K_zoomIn;
    keyBindings["-"] = K_zoomOut;
    keyBindings["r"] = K_pageRefresh;
    keyBindings["gg"] = K_scrollTop;
    keyBindings["zi"] = K_zoomIn;
    keyBindings["gh"] = K_goHomePage;
    keyBindings["zo"] = K_zoomOut;
    keyBindings["gu"] = K_goPathUp;
    keyBindings["gs"] = K_viewSource;
    keyBindings["ZZ"] = K_exit;
    keyBindings["zz"] = K_zoomReset;
    keyBindings["v"] = K_showKeyboard;
    keyBindings[":"] = K_commandMode;
    keyBindings["B"] = K_addBookmark;

    initSortedKeyBinds();
}

void ApplicationUI::initSQLDB() {
    QFile().remove("data/vib.sqlite");
    this->db = new bb::data::SqlDataAccess("data/vib.sqlite");
    this->db->execute("CREATE TABLE IF NOT EXISTS history (title TEXT, url TEXT, domain TEXT);");
    this->db->execute("CREATE TABLE IF NOT EXISTS bookmarks (title TEXT, url TEXT, domain TEXT);");

    qDebug() << "made database!";
}

void ApplicationUI::clearHistory() {
    db->execute("DELETE FROM history;");
}

void ApplicationUI::initSortedKeyBinds()
{

    //Sort keybindings by length, longest to shortest
    //This allows SL_keyPressedHandler to check
    //if the typed command is part of a bigger one

    QList<QString> keys = keyBindings.keys();

    //find length of longest binding
    int maxlen = 0;
    for (int i = 0; i < keys.size(); i++) {
        if (keys.at(i).size() > maxlen)
            maxlen = keys.at(i).size();
    }

    //use maxlen to push the biggest bindings to front
    for (int a = 0; a <= maxlen; a++) {
        for (int b = 0; b < keys.size(); b++) {
            if (keys.at(b).size() == a) {
                QString tmp = keys.at(b);
                keys.removeAt(b);
                keys.push_front(tmp);
            }
        }
    }

    this->sortedKeyBinds = keys;

    //for (int i=0;i<sortedKeyBinds.length();i++)
    //    qDebug() << sortedKeyBinds[i];
}

void ApplicationUI::initConfig() {
    config.SearchEngineString = "http://duckduckgo.com/?q=%s";
    config.DesktopMode = false;
    config.ImagesEnabled = true;
    config.JavascriptEnabled = true;
    config.CookiesEnabled = true;
    config.PixelRatioModifier = 1.0f;
    config.HomePage = "file:///";

    //TODO: Now read from file and parse!
}

QString boolToQstr(bool b, QString _true, QString _false) {
    if (b)
        return _true;
    else
        return _false;
}

void ApplicationUI::configWriter() {
    qDebug() << "Writing config!";
    QFile cfile("data/vibrowserrc");

    if (!cfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed opening config file for write!";
    }

    QTextStream stream(&cfile);

    stream << "set searchenginestring=" << config.SearchEngineString <<"\n";
    stream << "set homepage=" << config.HomePage <<"\n";
    stream << "set " << boolToQstr(config.JavascriptEnabled, "", "no") << "js" << "\n";
    stream << "set " << boolToQstr(config.ImagesEnabled, "", "no") << "images" << "\n";
    stream << "set " << boolToQstr(config.CookiesEnabled, "", "no") << "cookies" << "\n";
    stream << "set " << boolToQstr(config.DesktopMode, "", "no") << "desktop" << "\n";
    stream << "set pixelratiomod=" << config.PixelRatioModifier <<"\n";
    cfile.close();
}

void ApplicationUI::configParser() {

    QFile cfile("data/vibrowserrc");
    if (!cfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "failed opening cfile for read!";
    }

    while (cfile.atEnd()) {
        QString line = QString(cfile.readLine());

        qDebug() << line;
        if (line.startsWith("set")) {
            setCommandConfig(line.split(" ")[1]);
        }
    }
}


void ApplicationUI::printSettingsToHtml() {
    QHash<QString, QString> chash;
    chash["searchenginestring"] = QString(config.SearchEngineString);
    chash["homepage"] = QString(config.HomePage);
    chash["cookies"] = boolToQstr(config.CookiesEnabled, "true", "false");
    chash["images"] = boolToQstr(config.ImagesEnabled, "true", "false");
    chash["js"] = boolToQstr(config.JavascriptEnabled, "true", "false");
    chash["desktop"] = boolToQstr(config.DesktopMode, "true", "false");
    chash["pixelratiomod"] = QString().setNum(config.PixelRatioModifier, 'f');

    QString htmlBuilder = "<html><body><h6>";
    QList<QString> keys = chash.keys();

    for (int i=0;i<keys.length();i++) {
        htmlBuilder.append(keys.at(i)).append(" -> ").append(chash[keys.at(i)]).append("<br>");
    }

    htmlBuilder += "</h6></body></html>";

    currentTab->tabwebview->setHtml(htmlBuilder, QUrl("about:config"));
}
