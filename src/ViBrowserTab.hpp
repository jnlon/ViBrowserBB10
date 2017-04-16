/*
 * ViBrowserTab.hpp
 *
 *  Created on: Jul 12, 2015
 *      Author: jasp
 */

#ifndef ViBrowserTab_HPP_
#define ViBrowserTab_HPP_

#include <QObject>
#include <QDebug>
#include <bb/cascades/KeyEvent>
#include <bb/cascades/KeyListener>
#include <bb/cascades/Page>
#include <bb/cascades/Tab>
#include <bb/cascades/Label>
#include <bb/cascades/ScrollView>
#include <bb/cascades/WebView>
#include <bb/cascades/TextField>

#include "applicationui.hpp"

using namespace bb::cascades;

class ViBrowserTab : public QObject
{
    Q_OBJECT
public:
    ViBrowserTab(ApplicationUI *appui);
    virtual ~ViBrowserTab() {};

    ScrollView *tabscrollview;
    WebView *tabwebview;
    Container *tabcontainer;
    TextField *tabhintbar;
    TextField *taburlbar;
    TextField *tabtextsearchbar;
    TextField *tabwebsearchbar;
    TextField *tabcommandbar;
    TextField *tabinputbars[5];
    Label *tabtitlebar;
    Label *tabselectcounter;

    void initTextFieldToggles(TextField *bars[], ApplicationUI *appui);
};



#endif /* WEBTABVIEW_H_ */
