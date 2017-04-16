
#include "ViBrowserTab.hpp"
#include "applicationui.hpp"
#include <QObject>
#include <QDebug>

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>
#include <bb/cascades/controls/webview.h>
#include <bb/cascades/controls/scrollview.h>
#include <bb/cascades/WebSettings>
#include <bb/cascades/Container>
#include <bb/cascades/ScrollViewProperties>
#include <bb/cascades/StackLayout>
#include <bb/cascades/TextStyle>
#include <bb/cascades/Label>
#include <bb/cascades/KeyListener>
#include <bb/cascades/ChromeVisibility>
#include <bb/cascades/KeyEvent>
#include <bb/cascades/Tab>
#include <bb/cascades/Color>
#include <bb/cascades/StackLayoutProperties>
#include <bb/cascades/controls/container.h>
#include <bb/cascades/WebLoadRequest>

//hax

#include <bb/cascades/LabelTextFitMode>
#include <bb/cascades/LabelTextFitProperties>

using namespace bb::cascades;



ViBrowserTab::ViBrowserTab(ApplicationUI *appui) {

    //UI structure:
    //Page -> Container { Container [stacklayout] { Label (titlebar), Label (tabcounter) } ScrollView -> webview, TextField[s] (*bars) }
    //Switching tabs is just switching the first container.

    //Also, multiple TextFields are set up here (one for command, hint, url, search)
    //The alternative is using one TextField but having it juggle a bunch of jobs,
    //which means having to put qobject connect and disconnect everywhere,
    //which I'd rather avoid

    qDebug() << "ViBrowserTab Constructor!";

    Container *container = new Container;

    container->setHorizontalAlignment(HorizontalAlignment::Fill);
    container->setVerticalAlignment(VerticalAlignment::Fill);

    Container *headcontainer = new Container;
    StackLayout *stack = new StackLayout;
    stack->setOrientation(LayoutOrientation::LeftToRight);

    headcontainer->setBackground(Color::Transparent);
    headcontainer->setLayout(stack);
    headcontainer->setHorizontalAlignment(HorizontalAlignment::Fill);

    TextStyle tabcounterstyle;
    tabcounterstyle.setColor(Color::Cyan);

    Label *tabcounter = Label::create().textStyle(tabcounterstyle);

    tabcounter->setText("?/?");
    tabcounter->setTopMargin(0.0f);
    tabcounter->setBottomMargin(0.0f);
    tabcounter->setLeftMargin(8.0f);
    tabcounter->setRightMargin(0.0f);

    //LabelTextFitProperties test= new LabelTextFitProperties;
    //test.setMode();
    Label *titlebar = Label::create().textFitMode(LabelTextFitMode::FitToBounds);
    titlebar->setText("ViBrowser");
    titlebar->setBottomMargin(0.0f);
    titlebar->setTopMargin(0.0f);
    titlebar->setLeftMargin(0.0f);
    titlebar->setRightMargin(0.0f);
    titlebar->setLayoutProperties(StackLayoutProperties::create().spaceQuota(999.0f));

    TextField *fields[5];

    //make 4 text fields fields: command, hint, textsearch, url, websearch
    for (int i=0;i<5;i++) {


        fields[i] = TextField::create()
            .visible(false)
            .clearButtonVisible(false)
            .submitKey(SubmitKey::EnterKey)
            .inputMode(TextFieldInputMode::Url)
            .bottomMargin(0.0f)
            .topMargin(0.0f);


        fields[i]->setFocusRetentionPolicyFlags(
                FocusRetentionPolicy::LoseToNonFocusable |
                FocusRetentionPolicy::LoseOnScroll |
                FocusRetentionPolicy::LoseToFocusable );
    }


    ScrollView *scrollview = ScrollView::create()
        .scrollMode(ScrollMode::Both)
        .pinchToZoomEnabled(true)
        .builtInShortcutsEnabled(false)
        .overScrollEffectMode(OverScrollEffectMode::None);

    scrollview->scrollViewProperties()->setMaxContentScale(5.0f);
    scrollview->scrollViewProperties()->setMinContentScale(1.0f);
    scrollview->setHorizontalAlignment(HorizontalAlignment::Fill);
    scrollview->setVerticalAlignment(VerticalAlignment::Fill);

    WebView *webview = WebView::create();

    webview->setHorizontalAlignment(HorizontalAlignment::Fill);
    webview->setVerticalAlignment(VerticalAlignment::Fill);
    webview->setPreferredHeight(webview->maxHeight());
    webview->setPreferredWidth(webview->maxWidth());
    webview->setUrl(QUrl(appui->config.HomePage));
    webview->settings()->setZoomToFitEnabled(true);
    webview->settings()->setTextAutosizingEnabled(true);
    webview->settings()->setZoomToFitEnabled(true);

    //load configuration
    float dpr = webview->settings()->devicePixelRatio();
    webview->settings()->setDevicePixelRatio(dpr * appui->config.PixelRatioModifier);
    webview->settings()->setTextAutosizingEnabled(!appui->config.DesktopMode);
    webview->settings()->setCookiesEnabled(appui->config.CookiesEnabled);
    webview->settings()->setImageDownloadingEnabled(appui->config.ImagesEnabled);
    webview->settings()->setJavaScriptEnabled(appui->config.JavascriptEnabled);

    KeyListener* keylistener = KeyListener::create()
        .onKeyPressed(appui, SLOT(SL_onKeyPressedHandler(bb::cascades::KeyEvent *)))
        .onKeyLongPressed(appui, SLOT(SL_onKeyLongPressedHandler(bb::cascades::KeyEvent *)));

    webview->addKeyListener(keylistener);

    //Make the structure
    scrollview->setContent(webview);

    headcontainer->add(titlebar);
    headcontainer->add(tabcounter);

    container->add(headcontainer);
    container->add(scrollview);
    for (int i=0;i<5;i++) {
        container->add(fields[i]);
        this->tabinputbars[i] = fields[i];
    }

    //Class globals for accessibility
    this->tabscrollview = scrollview;
    this->tabwebview = webview;
    this->tabcontainer = container;
    this->tabtitlebar = titlebar;
    this->tabselectcounter = tabcounter;
    this->tabhintbar = fields[0];
    this->taburlbar = fields[1];
    this->tabwebsearchbar = fields[2];
    this->tabcommandbar = fields[3];
    this->tabtextsearchbar = fields[4];

    tabhintbar->setInputMode(TextFieldInputMode::Text);

    tabhintbar->setHintText("Link Text/Tag Number");
    taburlbar->setHintText("URL");
    tabcommandbar->setHintText("Command (see 'help commands')");
    tabtextsearchbar->setHintText("Search Webpage");
    tabwebsearchbar->setHintText("Search Engine Query");

    initTextFieldToggles(fields, appui);

    //This awful looking mess just specifies what happens when they hit enter
    //on any of the bars. Unfortunately, MOC requires a specific namespace
    QObject::connect(tabhintbar->input(),
            SIGNAL(submitted(bb::cascades::AbstractTextControl*)), appui,
            SLOT(SL_selectHint(bb::cascades::AbstractTextControl*)));

    QObject::connect(taburlbar->input(),
            SIGNAL(submitted(bb::cascades::AbstractTextControl*)), appui,
            SLOT(SL_goURL(bb::cascades::AbstractTextControl*)));

    QObject::connect(tabcommandbar->input(),
            SIGNAL(submitted(bb::cascades::AbstractTextControl*)), appui,
            SLOT(SL_runCommand(bb::cascades::AbstractTextControl*)));

    QObject::connect(tabtextsearchbar->input(),
            SIGNAL(submitted(bb::cascades::AbstractTextControl*)), appui,
            SLOT(SL_searchPageText(bb::cascades::AbstractTextControl*)));

    QObject::connect(tabwebsearchbar->input(),
            SIGNAL(submitted(bb::cascades::AbstractTextControl*)), appui,
            SLOT(SL_searchEngineQuery(bb::cascades::AbstractTextControl*)));

    //Communicate with js hint filter
    QObject::connect(tabhintbar,
            SIGNAL(textChanging(QString)),
            appui,
            SLOT(SL_updateHintFilter(QString)));

    QObject::connect(tabhintbar,
            SIGNAL(focusedChanged(bool)),
            appui,
            SLOT(SL_stopHintMode(bool)));

    //Recieve links from js
    QObject::connect(tabwebview,
            SIGNAL(messageReceived(QVariantMap)),
            appui,
            SLOT(SL_receiveHintsFromJS(QVariantMap)));

    //keep titlebar in sync with webview
    QObject::connect(tabwebview,
            SIGNAL(urlChanged(QUrl)),
            appui,
            SLOT(SL_updateUrl(QUrl)));

    //Used to inject js on page load, add history
    QObject::connect(tabwebview,
            SIGNAL(loadingChanged(bb::cascades::WebLoadRequest*)),
            appui,
            SLOT(SL_pageLoad(bb::cascades::WebLoadRequest*)));

}

void ViBrowserTab::initTextFieldToggles(TextField *bars[], ApplicationUI *appui) {

    qDebug() << "init toggles!";

    for (int i=0;i<5;i++) {

        KeyListener* keylistener = KeyListener::create()
                .onKeyPressed(appui, SLOT(SL_onBarKeyPressed(bb::cascades::KeyEvent *)));

        bars[i]->addKeyListener(keylistener);

        //When focus changes, change visibility
        //this causes the following code to hide it
        QObject::connect(bars[i],
                SIGNAL(focusedChanged(bool)),
                bars[i],
                SLOT(setVisible(bool)));

        //on submit, focus webpage.
        QObject::connect(bars[i]->input(),
                SIGNAL(submitted(bb::cascades::AbstractTextControl*)),
                tabwebview,
                SLOT(requestFocus()));

        //lose focus on screen touch
        QObject::connect(bars[i],
                SIGNAL(touchExit(bb::cascades::TouchExitEvent*)),
                bars[i],
                SLOT(loseFocus()));
    }
}
