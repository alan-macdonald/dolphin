/*
 * SPDX-FileCopyrightText: 2006-2010 Peter Penz <peter.penz19@gmail.com>
 * SPDX-FileCopyrightText: 2006 Gregor Kališnik <gregor@podnapisi.net>
 * SPDX-FileCopyrightText: 2012 Stuart Citrin <ctrn3e8@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "filterbar.h"

#include <KLocalizedString>

#include <QApplication>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QToolButton>

FilterBar::FilterBar(QWidget *parent)
    : AnimatedHeightWidget{parent}
{
    QWidget *contentsContainer = prepareContentsContainer();

    // Create button to lock text when changing folders
    m_lockButton = new QToolButton(contentsContainer);
    m_lockButton->setAutoRaise(true);
    m_lockButton->setCheckable(true);
    m_lockButton->setIcon(QIcon::fromTheme(QStringLiteral("object-unlocked")));
    m_lockButton->setToolTip(i18nc("@info:tooltip", "Keep Filter When Changing Folders"));
    connect(m_lockButton, &QToolButton::toggled, this, &FilterBar::slotToggleLockButton);

    // Create filter editor
    m_filterInput = new QLineEdit(contentsContainer);
    m_filterInput->setLayoutDirection(Qt::LeftToRight);
    m_filterInput->setClearButtonEnabled(true);
    m_filterInput->setPlaceholderText(i18n("Filter…"));
    connect(m_filterInput, &QLineEdit::textChanged, this, &FilterBar::filterChanged);
    m_filterInput->installEventFilter(this);
    setFocusProxy(m_filterInput);

    // Create close button
    QToolButton *closeButton = new QToolButton(contentsContainer);
    closeButton->setAutoRaise(true);
    closeButton->setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));
    closeButton->setToolTip(i18nc("@info:tooltip", "Hide Filter Bar"));
    connect(closeButton, &QToolButton::clicked, this, &FilterBar::closeRequest);

    // Apply layout
    QHBoxLayout *hLayout = new QHBoxLayout(contentsContainer);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(m_lockButton);
    hLayout->addWidget(m_filterInput);
    hLayout->addWidget(closeButton);

    setTabOrder(m_lockButton, closeButton);
    setTabOrder(closeButton, m_filterInput);
}

FilterBar::~FilterBar()
{
}

void FilterBar::closeFilterBar()
{
    setVisible(false, WithAnimation);
    clear();
    if (m_lockButton) {
        m_lockButton->setChecked(false);
    }
}

void FilterBar::selectAll()
{
    m_filterInput->selectAll();
}

void FilterBar::clear()
{
    m_filterInput->clear();
}

void FilterBar::clearIfUnlocked()
{
    if (!m_lockButton || !(m_lockButton->isChecked())) {
        clear();
    }
}

void FilterBar::slotToggleLockButton(bool checked)
{
    if (checked) {
        m_lockButton->setIcon(QIcon::fromTheme(QStringLiteral("object-locked")));
    } else {
        m_lockButton->setIcon(QIcon::fromTheme(QStringLiteral("object-unlocked")));
        clear();
    }
}

bool FilterBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_filterInput && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        switch (keyEvent->key()) {
            case Qt::Key_Escape:
                if (m_closeOnEscape || m_filterInput->text().isEmpty()) {
                    Q_EMIT closeRequest();
                } else {
                    m_filterInput->clear();
                }
                return true;

            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Down:
            case Qt::Key_PageDown:
            case Qt::Key_Up:
            case Qt::Key_PageUp: {
                // 1) Ask the container to give focus to the view
                Q_EMIT focusViewRequest();

                // 2) Now the view (or its internal item view) should be the focus widget.
                QWidget *target = QApplication::focusWidget();

                // 3) Re-post the same key event to the view so it handles navigation/activation.
                if (target && target != m_filterInput) {
                    auto *forwarded = new QKeyEvent(
                        keyEvent->type(),
                        keyEvent->key(),
                        keyEvent->modifiers(),
                        keyEvent->text(),
                        keyEvent->isAutoRepeat(),
                        keyEvent->count());
                    QApplication::postEvent(target, forwarded);

                    m_filterInput->setFocus();
                }

                return true;
            }

            default:
                break;
        }
    }

    return AnimatedHeightWidget::eventFilter(watched, event);
}

void FilterBar::setCloseOnEscape(bool enabled)
{
    m_closeOnEscape = enabled;
}


void FilterBar::showEvent(QShowEvent *event)
{
    if (!event->spontaneous()) {
        m_filterInput->setFocus();
    }
}

void FilterBar::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Escape:
            if (m_closeOnEscape || m_filterInput->text().isEmpty()) {
                Q_EMIT closeRequest();
            } else {
                m_filterInput->clear();
            }
            return;

        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Down:
        case Qt::Key_PageDown:
        case Qt::Key_Up:
        case Qt::Key_PageUp: {
            Q_EMIT focusViewRequest();

            QWidget *target = QApplication::focusWidget();
            if (target && target != this && target != m_filterInput) {
                auto *forwarded = new QKeyEvent(
                    event->type(),
                    event->key(),
                    event->modifiers(),
                    event->text(),
                    event->isAutoRepeat(),
                    event->count());
                QApplication::postEvent(target, forwarded);
            }

            m_filterInput->setFocus();
            return;
        }

        default:
            break;
    }

    QWidget::keyPressEvent(event);
}


void FilterBar::appendToFilter(const QString& text)
{
    if (text.isEmpty()) {
        return;
    }

    // Find the line edit that actually holds the filter text.
    // Prefer a direct member if you have one; otherwise, locate it safely.
    QLineEdit* lineEdit = findChild<QLineEdit*>();
    if (!lineEdit) {
        return;
    }

    // If the user has selected all (common when the bar opens), replace selection.
    // Otherwise append.
    if (lineEdit->hasSelectedText() && lineEdit->selectedText() == lineEdit->text()) {
        lineEdit->clear();
    }

    lineEdit->insert(text);
}

int FilterBar::preferredHeight() const
{
    return std::max(m_filterInput->sizeHint().height(), m_lockButton->sizeHint().height());
}

#include "moc_filterbar.cpp"
