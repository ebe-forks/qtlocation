/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qplaceattribute_p.h"
#include "qplaceattribute.h"

#if !defined(Q_CC_MWERKS)
template<> QT_PREPEND_NAMESPACE(QPlaceAttributePrivate) *QSharedDataPointer<QT_PREPEND_NAMESPACE(QPlaceAttributePrivate)>::clone()
{
    return d->clone();
}
#endif

QT_USE_NAMESPACE


QPlaceAttributePrivate::QPlaceAttributePrivate(const QPlaceAttributePrivate &other)
    : QSharedData(other),
      label(other.label),
      text(other.text)
{
}

bool QPlaceAttributePrivate::operator== (const QPlaceAttributePrivate &other) const
{
    return label == other.label
            && text == other.text;
}

/*!
\class QPlaceAttribute
\brief The QPlaceAttribute class represents generic attribute information about a place.
\inmodule QtLocation

\ingroup location
\ingroup places-main
*/

/*!
    Constructs an attribute.
*/
QPlaceAttribute::QPlaceAttribute()
    : d_ptr(new QPlaceAttributePrivate)
{
}

/*!
    Destroys the attribute.
*/
QPlaceAttribute::~QPlaceAttribute()
{
}

/*!
    Creates a copy of \a other.
*/
QPlaceAttribute::QPlaceAttribute(const QPlaceAttribute &other)
    :d_ptr(other.d_ptr)
{
}

/*!
    Assigns \a other to this attribute and returns a reference to this
    attribute.
*/
QPlaceAttribute &QPlaceAttribute::operator=(const QPlaceAttribute &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

/*!
    Returns true if \a other is equal to this attribute, otherwise
    returns false.
*/
bool QPlaceAttribute::operator== (const QPlaceAttribute &other) const
{
    if (d_ptr == other.d_ptr)
        return true;
    return ( *(d_ptr.constData()) == *(other.d_ptr.constData()));
}

/*!
    Returns true if \a other is not equal to this place,
    otherwise returns false.
*/
bool QPlaceAttribute::operator!= (const QPlaceAttribute &other) const
{
    return (!this->operator ==(other));
}

/*!
    Returns a user-visible label describing the attribute.
*/
QString QPlaceAttribute::label() const
{
    return d_ptr->label;
}

/*!
    Sets the label of the attribute.
*/
void QPlaceAttribute::setLabel(const QString &label)
{
    d_ptr->label = label;
}

/*!
    Returns text representing the attribute value.
*/
QString QPlaceAttribute::text() const
{
    return d_ptr->text;
}

/*!
    Sets the text of the attribute.
*/
void QPlaceAttribute::setText(const QString &text)
{
    d_ptr->text = text;
}
