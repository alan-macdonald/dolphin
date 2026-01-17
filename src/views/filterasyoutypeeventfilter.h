#pragma once

#include <QObject>
#include <QPointer>

class QWidget;
class QKeyEvent;

class FilterAsYouTypeEventFilter final : public QObject
{
    Q_OBJECT

public:
    explicit FilterAsYouTypeEventFilter(QWidget *viewWidget, QObject *parent = nullptr);

    void setEnabled(bool enabled);
    bool isEnabled() const;

Q_SIGNALS:
    void startFilteringRequested(const QString &initialText);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    bool shouldHandleKeyPress(QKeyEvent *keyEvent) const;
    static bool isAlphaNumericAscii(const QString &text);
    static bool isEditableWidget(const QWidget *widget);

    QPointer<QWidget> m_viewWidget;
    bool m_enabled = false;
};
