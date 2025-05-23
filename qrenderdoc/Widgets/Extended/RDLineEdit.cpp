/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2025 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include "RDLineEdit.h"
#include <QKeyEvent>
#include "Code/Interface/QRDInterface.h"

RDLineEdit::RDLineEdit(QWidget *parent) : QLineEdit(parent)
{
}

RDLineEdit::~RDLineEdit()
{
}

void RDLineEdit::focusInEvent(QFocusEvent *e)
{
  QLineEdit::focusInEvent(e);
  emit(enter());
}

void RDLineEdit::focusOutEvent(QFocusEvent *e)
{
  QLineEdit::focusOutEvent(e);
  emit(leave());
}

void RDLineEdit::keyPressEvent(QKeyEvent *e)
{
  QLineEdit::keyPressEvent(e);
  emit(keyPress(e));
}

bool RDLineEdit::event(QEvent *e)
{
  if(m_acceptTabs && e->type() == QEvent::KeyPress)
  {
    QKeyEvent *ke = (QKeyEvent *)e;
    if(ke->key() == Qt::Key_Tab)
    {
      keyPressEvent(ke);
      e->accept();
      return true;
    }
  }
  return QLineEdit::event(e);
}
