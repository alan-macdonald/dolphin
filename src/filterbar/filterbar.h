/*
 * SPDX-FileCopyrightText: 2006-2010 Peter Penz <peter.penz19@gmail.com>
 * SPDX-FileCopyrightText: 2006 Gregor Kališnik <gregor@podnapisi.net>
 * SPDX-FileCopyrightText: 2012 Stuart Citrin <ctrn3e8@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FILTERBAR_H
#define FILTERBAR_H

#include "animatedheightwidget.h"

class QLineEdit;
class QToolButton;

/**
 * @brief Provides an input field for filtering the currently shown items.
 *
 * @author Gregor Kališnik <gregor@podnapisi.net>
 */
class FilterBar : public AnimatedHeightWidget
{
    Q_OBJECT

public:
    explicit FilterBar(QWidget *parent = nullptr);
    ~FilterBar() override;

    /** Called by view container to hide this **/
    void closeFilterBar();

    /**
     * Selects the whole text of the filter bar.
     */
    void selectAll();

    /**
     * Append text to the current filter.
     */
    void appendToFilter(const QString& text);

    /*
     * Controls whether the filter bar should close immediately on escape, or should
     * first check if it contains any text and clear that first.
     */
    void setCloseOnEscape(bool enabled);

public Q_SLOTS:
    /** Clears the input field. */
    void clear();
    /** Clears the input field if the "lock button" is disabled. */
    void clearIfUnlocked();
    /** The input field is cleared also if the "lock button" is released. */
    void slotToggleLockButton(bool checked);

Q_SIGNALS:
    /**
     * Signal that reports the name filter has been
     * changed to \a nameFilter.
     */
    void filterChanged(const QString &nameFilter);

    /**
     * Emitted as soon as the filterbar should get closed.
     */
    void closeRequest();

    /*
     * Emitted as soon as the focus should be returned back to the view.
     */
    void focusViewRequest();

    /*
     * Request navigation in the view (up/down cursor keys).
    */
    void navigateInViewRequested(int key, Qt::KeyboardModifiers modifiers);

protected:
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    /** @see AnimatedHeightWidget::preferredHeight() */
    int preferredHeight() const override;

private:
    QLineEdit *m_filterInput;
    QToolButton *m_lockButton;
    bool m_closeOnEscape = false;
};

#endif
