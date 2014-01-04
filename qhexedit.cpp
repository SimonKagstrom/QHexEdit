#include "qhexedit.h"

QHexEdit::QHexEdit(QWidget *parent): QFrame(parent)
{
    this->_vscrollbar = new QScrollBar(Qt::Vertical);
    this->_scrollarea = new QScrollArea();
    this->_hexedit_p = new QHexEditPrivate(this->_scrollarea, this->_vscrollbar);

    /* Forward QHexEditPrivate's Signals */
    connect(this->_hexedit_p, SIGNAL(positionChanged(qint64)), this, SIGNAL(positionChanged(qint64)));
    connect(this->_hexedit_p, SIGNAL(selectionChanged(qint64)), this, SIGNAL(selectionChanged(qint64)));
    connect(this->_hexedit_p, SIGNAL(bytesChanged(qint64)), this, SIGNAL(bytesChanged(qint64)));
    connect(this->_hexedit_p, SIGNAL(verticalScrollBarValueChanged(int)), this, SIGNAL(verticalScrollBarValueChanged(int)));

    this->_scrollarea->setFocusPolicy(Qt::NoFocus);
    this->_scrollarea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); /* Do not show vertical QScrollBar!!! */
    this->_scrollarea->setFrameStyle(QFrame::NoFrame);
    this->_scrollarea->setWidgetResizable(true);
    this->_scrollarea->setWidget(this->_hexedit_p);

    this->setFocusPolicy(Qt::NoFocus);

    this->_hlayout = new QHBoxLayout();
    this->_hlayout->setSpacing(0);
    this->_hlayout->setMargin(0);
    this->_hlayout->addWidget(this->_scrollarea);
    this->_hlayout->addWidget(this->_vscrollbar);

    this->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    this->setLayout(this->_hlayout);
}

void QHexEdit::doAnd(qint64 start, qint64 end, uchar value)
{
    this->_hexedit_p->doAnd(start, end, value);
}

void QHexEdit::doOr(qint64 start, qint64 end, uchar value)
{
    this->_hexedit_p->doOr(start, end, value);
}

void QHexEdit::doXor(qint64 start, qint64 end, uchar value)
{
    this->_hexedit_p->doXor(start, end, value);
}

void QHexEdit::doMod(qint64 start, qint64 end, uchar value)
{
    this->_hexedit_p->doMod(start, end, value);
}

void QHexEdit::doNot(qint64 start, qint64 end)
{
    this->_hexedit_p->doNot(start, end);
}

void QHexEdit::undo()
{
    this->_hexedit_p->undo();
}

void QHexEdit::setData(QHexEditData *hexeditdata)
{
    this->_hexedit_p->setData(hexeditdata);
}

void QHexEdit::setSelection(qint64 start, qint64 end)
{
    this->_hexedit_p->setSelection(start, end);
}

void QHexEdit::setRangeColor(qint64 start, qint64 end, QColor color)
{
    this->_hexedit_p->setRangeColor(start, end, color);
}

void QHexEdit::removeRangeColor(qint64 start, qint64 end)
{
    this->_hexedit_p->removeRangeColor(start, end);
}

void QHexEdit::resetRangeColor()
{
    this->_hexedit_p->resetRangeColor();
}

void QHexEdit::setVerticalScrollBarValue(int value)
{
    this->_hexedit_p->setVerticalScrollBarValue(value);
}

void QHexEdit::setCursorPos(qint64 pos)
{
    this->_hexedit_p->setCursorPos(pos);
}

void QHexEdit::paste()
{
    this->_hexedit_p->paste();
}

void QHexEdit::selectAll()
{
    this->_hexedit_p->setSelection(0, -1);
}

int QHexEdit::addressWidth()
{
    return this->_hexedit_p->addressWidth();
}

QHexEditData *QHexEdit::data()
{
    return this->_hexedit_p->data();
}

qint64 QHexEdit::indexOf(QByteArray &ba, qint64 start)
{
    return this->_hexedit_p->indexOf(ba, start);
}

qint64 QHexEdit::cursorPos()
{
    return this->_hexedit_p->cursorPos();
}

qint64 QHexEdit::selectionStart()
{
    return this->_hexedit_p->selectionStart();
}

qint64 QHexEdit::selectionEnd()
{
    return this->_hexedit_p->selectionEnd();
}

bool QHexEdit::readOnly()
{
    return this->_hexedit_p->readOnly();
}

void QHexEdit::setReadOnly(bool b)
{
    this->_hexedit_p->setReadOnly(b);
}

void QHexEdit::copy()
{
    this->_hexedit_p->copy();
}

void QHexEdit::cut()
{
    this->_hexedit_p->cut();
}

void QHexEdit::redo()
{
    this->_hexedit_p->redo();
}

void QHexEdit::setFont(const QFont &font)
{
	this->_hexedit_p->setFont(font);
}
