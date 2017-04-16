
#include <bb/cascades/ScrollView>
#include "applicationui.hpp"

using namespace bb::cascades;

void ApplicationUI::scrollDown(float percent, ScrollView *view) {

    float scaleOffset = view->contentScale();
    float y = view->viewableArea().top()/scaleOffset;
    float x = view->viewableArea().left()/scaleOffset;

    float toScroll = (view->viewableArea().height()/100.0f)*percent;

    view->scrollToPoint(x, y+toScroll, ScrollAnimation::None);
}

void ApplicationUI::zoomReset(ScrollView *scrollview) {
    scrollview->resetViewableArea(ScrollAnimation::None);
}


void ApplicationUI::zoomIn(float percent, ScrollView *view) {

    float currentScale = view->contentScale();
    QRect viewRect = view->viewableArea().toRect();

    /* zoomToRect expects the coordinates to ignore scale,
     * so here we undo its effect */
    float top = viewRect.top()/currentScale;
    float left = viewRect.left()/currentScale;

    float newWidth = viewRect.width() - (viewRect.width() / 100 * percent);
    float newHeight = viewRect.height() - (viewRect.height() / 100 * percent);

    viewRect.setRect(left, top, newWidth, newHeight);

    view->zoomToRect(viewRect, ScrollAnimation::None);
}


void ApplicationUI::scrollRight(float percent, ScrollView *view)
{

    float scaleOffset = view->contentScale();
    float y = view->viewableArea().top() / scaleOffset;
    float x = view->viewableArea().left() / scaleOffset;

    float toScroll = (view->viewableArea().width() / 100.0f) * percent;

    view->scrollToPoint(x + toScroll, y, ScrollAnimation::None);
}
