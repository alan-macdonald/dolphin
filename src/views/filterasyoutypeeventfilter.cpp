#include "filterasyoutypeeventfilter.h"

#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QTextEdit>

FilterAsYouTypeEventFilter::FilterAsYouTypeEventFilter(QWidget *viewWidget, QObject *parent)
    : QObject(parent)
    , m_viewWidget(viewWidget)
{
    // Listen globally, then restrict by checking focus widget + parent chain.
    if (qApp) {
        qApp->installEventFilter(this);
    }
}

void FilterAsYouTypeEventFilter::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool FilterAsYouTypeEventFilter::isEnabled() const
{
    return m_enabled;
}

bool FilterAsYouTypeEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);

    if (!m_enabled || !m_viewWidget) {
        return QObject::eventFilter(watched, event);
    }

    if (event->type() != QEvent::KeyPress) {
        return QObject::eventFilter(watched, event);
    }

    auto *keyEvent = static_cast<QKeyEvent *>(event);
    if (!shouldHandleKeyPress(keyEvent)) {
        return QObject::eventFilter(watched, event);
    }

    const QString text = keyEvent->text();
    if (!isAlphaNumericAscii(text)) {
        return QObject::eventFilter(watched, event);
    }

    Q_EMIT startFilteringRequested(text);
    return true; // stop default "type-to-select" behavior
}

bool FilterAsYouTypeEventFilter::shouldHandleKeyPress(QKeyEvent *keyEvent) const
{
    if (!keyEvent) {
        return false;
    }

    if (keyEvent->isAutoRepeat()) {
        return false;
    }

    const Qt::KeyboardModifiers mods = keyEvent->modifiers();
    const bool hasDisallowedModifier =
        mods.testFlag(Qt::ControlModifier) ||
        mods.testFlag(Qt::AltModifier) ||
        mods.testFlag(Qt::MetaModifier);
    if (hasDisallowedModifier) {
        return false;
    }

    QWidget *focusWidget = QApplication::focusWidget();
    if (!focusWidget) {
        return false;
    }

    // Don’t steal input from text / spin / combo editors.
    if (isEditableWidget(focusWidget)) {
        return false;
    }

    // Only react when the focus is inside this view’s widget subtree.
    bool insideThisView = false;
    for (QWidget *w = focusWidget; w; w = w->parentWidget()) {
        if (w == m_viewWidget) {
            insideThisView = true;
            break;
        }
    }
    if (!insideThisView) {
        return false;
    }

    if (keyEvent->text().isEmpty()) {
        return false;
    }

    return true;
}

bool FilterAsYouTypeEventFilter::isAlphaNumericAscii(const QString &text)
{
    if (text.size() != 1) {
        return false;
    }

    const ushort u = text.at(0).unicode();

    const bool isDigit = (u >= static_cast<ushort>('0') && u <= static_cast<ushort>('9'));
    const bool isUpper = (u >= static_cast<ushort>('A') && u <= static_cast<ushort>('Z'));
    const bool isLower = (u >= static_cast<ushort>('a') && u <= static_cast<ushort>('z'));

    return isDigit || isUpper || isLower;
}

bool FilterAsYouTypeEventFilter::isEditableWidget(const QWidget *widget)
{
    if (!widget) {
        return false;
    }

    if (qobject_cast<const QLineEdit *>(widget) ||
        qobject_cast<const QTextEdit *>(widget) ||
        qobject_cast<const QPlainTextEdit *>(widget) ||
        qobject_cast<const QSpinBox *>(widget) ||
        qobject_cast<const QComboBox *>(widget)) {
        return true;
    }

    return widget->testAttribute(Qt::WA_InputMethodEnabled);
}
