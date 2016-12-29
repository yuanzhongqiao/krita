/* This file is part of the KDE project
 * Copyright (C) 2009 Jos van den Oever <jos@vandenoever.info>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoFlake.h"
#include "KoShape.h"

#include <QGradient>
#include <math.h>

QGradient *KoFlake::cloneGradient(const QGradient *gradient)
{
    if (! gradient)
        return 0;

    QGradient *clone = 0;

    switch (gradient->type()) {
    case QGradient::LinearGradient:
    {
        const QLinearGradient *lg = static_cast<const QLinearGradient*>(gradient);
        clone = new QLinearGradient(lg->start(), lg->finalStop());
        break;
    }
    case QGradient::RadialGradient:
    {
        const QRadialGradient *rg = static_cast<const QRadialGradient*>(gradient);
        clone = new QRadialGradient(rg->center(), rg->radius(), rg->focalPoint());
        break;
    }
    case QGradient::ConicalGradient:
    {
        const QConicalGradient *cg = static_cast<const QConicalGradient*>(gradient);
        clone = new QConicalGradient(cg->center(), cg->angle());
        break;
    }
    default:
        return 0;
    }

    clone->setCoordinateMode(gradient->coordinateMode());
    clone->setSpread(gradient->spread());
    clone->setStops(gradient->stops());

    return clone;
}

QPointF KoFlake::toRelative(const QPointF &absolute, const QSizeF &size)
{
    return QPointF(size.width() == 0 ? 0: absolute.x() / size.width(),
                   size.height() == 0 ? 0: absolute.y() / size.height());
}

QPointF KoFlake::toAbsolute(const QPointF &relative, const QSizeF &size)
{
    return QPointF(relative.x() * size.width(), relative.y() * size.height());
}

#include <KoShape.h>
#include <QTransform>
#include "kis_debug.h"
#include "kis_algebra_2d.h"

void KoFlake::resizeShape(KoShape *shape, qreal scaleX, qreal scaleY,
                          const QPointF &absoluteStillPoint,
                          bool usePostScaling, const QTransform &postScalingCoveringTransform)
{
    QPointF localStillPoint = shape->absoluteTransformation(0).inverted().map(absoluteStillPoint);

    QPointF relativeStillPoint = KisAlgebra2D::absoluteToRelative(localStillPoint, shape->outlineRect());
    QPointF parentalStillPointBefore = shape->transformation().map(localStillPoint);

    if (usePostScaling) {
        const QTransform scale = QTransform::fromScale(scaleX, scaleY);
        shape->setTransformation(shape->transformation() *
                                 postScalingCoveringTransform.inverted() *
                                 scale * postScalingCoveringTransform);
    } else {
        using namespace KisAlgebra2D;

        const QSizeF oldSize(shape->size());
        const QSizeF newSize(oldSize.width() * qAbs(scaleX), oldSize.height() * qAbs(scaleY));

        const QTransform mirrorTransform = QTransform::fromScale(signPZ(scaleX), signPZ(scaleY));

        shape->setSize(newSize);

        if (!mirrorTransform.isIdentity()) {
            shape->setTransformation(mirrorTransform * shape->transformation());
        }

    }

    QPointF newLocalStillPoint = KisAlgebra2D::relativeToAbsolute(relativeStillPoint, shape->outlineRect());
    QPointF parentalStillPointAfter = shape->transformation().map(newLocalStillPoint);

    QPointF diff = parentalStillPointBefore - parentalStillPointAfter;
    shape->setTransformation(shape->transformation() * QTransform::fromTranslate(diff.x(), diff.y()));
}

QPointF KoFlake::anchorToPoint(AnchorPosition anchor, const QRectF rect, bool *valid)
{
    static QVector<QPointF> anchorTable;

    if (anchorTable.isEmpty()) {
        anchorTable << QPointF(0.0,0.0);
        anchorTable << QPointF(0.5,0.0);
        anchorTable << QPointF(1.0,0.0);

        anchorTable << QPointF(0.0,0.5);
        anchorTable << QPointF(0.5,0.5);
        anchorTable << QPointF(1.0,0.5);

        anchorTable << QPointF(0.0,1.0);
        anchorTable << QPointF(0.5,1.0);
        anchorTable << QPointF(1.0,1.0);
    }

    if (anchor == NoAnchor) {
        if (valid) {
            *valid = false;
        }
        return rect.topLeft();
    } else if (valid) {
        *valid = true;
    }

    return KisAlgebra2D::relativeToAbsolute(anchorTable[int(anchor)], rect);
}
