/* This file is part of the KDE project
 * Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include "DefaultToolWidget.h"
#include "DefaultTool.h"

#include <KoInteractionTool.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceManager.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoUnit.h>
#include <commands/KoShapeResizeCommand.h>
#include <commands/KoShapeMoveCommand.h>
#include <commands/KoShapeSizeCommand.h>
#include <commands/KoShapeTransformCommand.h>
#include <commands/KoShapeKeepAspectRatioCommand.h>
#include <KoPositionSelector.h>
#include "SelectionDecorator.h"

#include "KoAnchorSelectionWidget.h"

#include <QAction>
#include <QSize>
#include <QRadioButton>
#include <QLabel>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QList>
#include <QTransform>
#include <kis_algebra_2d.h>

#include "kis_aspect_ratio_locker.h"
#include "kis_debug.h"
#include "kis_acyclic_signal_connector.h"

DefaultToolWidget::DefaultToolWidget(KoInteractionTool *tool, QWidget *parent)
    : QWidget(parent)
    , m_tool(tool)
    , m_sizeAspectLocker(new KisAspectRatioLocker())
{
    setupUi(this);

    setUnit(m_tool->canvas()->unit());

    // Connect and initialize automated aspect locker
    m_sizeAspectLocker->connectSpinBoxes(widthSpinBox, heightSpinBox, aspectButton);
    aspectButton->setKeepAspectRatio(false);


    // TODO: use valueChanged() instead!
    connect(positionXSpinBox, SIGNAL(editingFinished()), this, SLOT(slotRepositionShapes()));
    connect(positionYSpinBox, SIGNAL(editingFinished()), this, SLOT(slotRepositionShapes()));

    // TODO: use valueChanged() instead!
    connect(widthSpinBox, SIGNAL(editingFinished()), this, SLOT(slotResizeShapes()));
    connect(heightSpinBox, SIGNAL(editingFinished()), this, SLOT(slotResizeShapes()));

    KoSelection *selection = m_tool->canvas()->shapeManager()->selection();
    connect(selection, SIGNAL(selectionChanged()), this, SLOT(slotUpdatePositionBoxes()));
    connect(selection, SIGNAL(selectionChanged()), this, SLOT(slotUpdateSizeBoxes()));
    connect(selection, SIGNAL(selectionChanged()), this, SLOT(slotUpdateCheckboxes()));

    KoShapeManager *manager = m_tool->canvas()->shapeManager();
    connect(manager, SIGNAL(selectionContentChanged()), this, SLOT(slotUpdatePositionBoxes()));
    connect(manager, SIGNAL(selectionContentChanged()), this, SLOT(slotUpdateSizeBoxes()));

    connect(chkGlobalCoordinates, SIGNAL(toggled(bool)), SLOT(slotUpdateSizeBoxes()));

    KisAcyclicSignalConnector *acyclicConnector = new KisAcyclicSignalConnector(this);
    acyclicConnector->connectForwardVoid(m_sizeAspectLocker.data(), SIGNAL(aspectButtonChanged()), this, SLOT(slotAspectButtonToggled()));
    acyclicConnector->connectBackwardVoid(selection, SIGNAL(selectionChanged()), this, SLOT(slotUpdateAspectButton()));
    acyclicConnector->connectBackwardVoid(manager, SIGNAL(selectionContentChanged()), this, SLOT(slotUpdateAspectButton()));


    // Connect and initialize anchor point resource
    KoCanvasResourceManager *resourceManager = m_tool->canvas()->resourceManager();
    connect(resourceManager,
            SIGNAL(canvasResourceChanged(int,QVariant)),
            SLOT(resourceChanged(int,QVariant)));
    resourceManager->setResource(DefaultTool::HotPosition, int(KoFlake::Center));

    // Connect anchor point selector
    connect(positionSelector, SIGNAL(valueChanged(KoFlake::AnchorPosition)), SLOT(slotAnchorPointChanged()));
}

DefaultToolWidget::~DefaultToolWidget()
{
}

#include <functional>

// FIXME: remove code duplication with KritaUtils!
template <class C>
    void filterContainer(C &container, std::function<bool(typename C::reference)> keepIf) {

        auto newEnd = std::remove_if(container.begin(), container.end(), std::unary_negate<decltype(keepIf)>(keepIf));
        while (newEnd != container.end()) {
           newEnd = container.erase(newEnd);
        }
}

void tryAnchorPosition(KoFlake::AnchorPosition anchor,
                       const QRectF &rect,
                       QPointF *position)
{
    bool valid = false;
    QPointF anchoredPosition = KoFlake::anchorToPoint(anchor, rect, &valid);

    if (valid) {
        *position = anchoredPosition;
    }
}

QList<KoShape*> fetchEditableShapes(KoSelection *selection)
{
    QList<KoShape*> shapes = selection->selectedShapes();

    filterContainer (shapes, [](KoShape *shape) {
        return shape->isEditable();
    });

    return shapes;
}

QRectF calculateSelectionBounds(KoSelection *selection,
                                KoFlake::AnchorPosition anchor,
                                bool useGlobalSize,
                                QList<KoShape*> *outShapes = 0)
{
    QList<KoShape*> shapes = fetchEditableShapes(selection);

    KoShape *shape = shapes.size() == 1 ? shapes.first() : selection;

    QRectF resultRect = shape->outlineRect();

    QPointF resultPoint = resultRect.topLeft();
    tryAnchorPosition(anchor, resultRect, &resultPoint);

    if (useGlobalSize) {
        resultRect = shape->absoluteTransformation(0).mapRect(resultRect);
    } else {
        /**
         * Some shapes, e.g. KoSelection and KoShapeGroup don't have real size() and
         * do all the resizing with transformation(), just try to cover this case and
         * fetch their scale using the transform.
         */

        KisAlgebra2D::DecomposedMatix matrix(shape->transformation());
        resultRect = matrix.scaleTransform().mapRect(resultRect);
    }

    resultPoint = shape->absoluteTransformation(0).map(resultPoint);

    if (outShapes) {
        *outShapes = shapes;
    }

    return QRectF(resultPoint, resultRect.size());
}

void DefaultToolWidget::slotAnchorPointChanged()
{
    QVariant newValue(positionSelector->value());
    m_tool->canvas()->resourceManager()->setResource(DefaultTool::HotPosition, newValue);
    slotUpdatePositionBoxes();
}

void DefaultToolWidget::slotUpdateCheckboxes()
{
    KoSelection *selection = m_tool->canvas()->shapeManager()->selection();
    QList<KoShape*> shapes = fetchEditableShapes(selection);

    chkUniformScaling->setEnabled(shapes.size() == 1);

    // TODO: not implemented yet!
    chkAnchorLock->setEnabled(false);
    chkGlobalCoordinates->setEnabled(false);
}

void DefaultToolWidget::slotAspectButtonToggled()
{
    KoSelection *selection = m_tool->canvas()->shapeManager()->selection();
    QList<KoShape*> shapes = fetchEditableShapes(selection);

    QList<bool> oldKeepAspectRatio;
    QList<bool> newKeepAspectRatio;

    Q_FOREACH (KoShape *shape, shapes) {
        oldKeepAspectRatio << shape->keepAspectRatio();
        newKeepAspectRatio << aspectButton->keepAspectRatio();
    }

    KUndo2Command *cmd =
        new KoShapeKeepAspectRatioCommand(shapes, oldKeepAspectRatio, newKeepAspectRatio);

    m_tool->canvas()->addCommand(cmd);
}

void DefaultToolWidget::slotUpdateAspectButton()
{
    KoSelection *selection = m_tool->canvas()->shapeManager()->selection();
    QList<KoShape*> shapes = fetchEditableShapes(selection);

    bool hasKeepAspectRatio = false;
    bool hasNotKeepAspectRatio = false;

    Q_FOREACH (KoShape *shape, shapes) {
        if (shape->keepAspectRatio()) {
            hasKeepAspectRatio = true;
        } else {
            hasNotKeepAspectRatio = true;
        }

        if (hasKeepAspectRatio && hasNotKeepAspectRatio) break;
    }

    Q_UNUSED(hasNotKeepAspectRatio); // TODO: use for tristated mode of the checkbox

    aspectButton->setKeepAspectRatio(hasKeepAspectRatio);
}

void DefaultToolWidget::slotUpdateSizeBoxes()
{
    const bool useGlobalSize = chkGlobalCoordinates->isChecked();
    const KoFlake::AnchorPosition anchor = positionSelector->value();

    KoSelection *selection = m_tool->canvas()->shapeManager()->selection();
    QRectF bounds = calculateSelectionBounds(selection, anchor, useGlobalSize);

    const bool hasSizeConfiguration = !bounds.isNull();

    widthSpinBox->setEnabled(hasSizeConfiguration);
    heightSpinBox->setEnabled(hasSizeConfiguration);

    if (hasSizeConfiguration) {
        widthSpinBox->changeValue(bounds.width());
        heightSpinBox->changeValue(bounds.height());
        m_sizeAspectLocker->updateAspect();
    }
}

void DefaultToolWidget::slotUpdatePositionBoxes()
{
    const bool useGlobalSize = chkGlobalCoordinates->isChecked();
    const KoFlake::AnchorPosition anchor = positionSelector->value();

    KoSelection *selection = m_tool->canvas()->shapeManager()->selection();
    QRectF bounds = calculateSelectionBounds(selection, anchor, useGlobalSize);

    const bool hasSizeConfiguration = !bounds.isNull();

    positionXSpinBox->setEnabled(hasSizeConfiguration);
    positionYSpinBox->setEnabled(hasSizeConfiguration);

    if (hasSizeConfiguration) {
        positionXSpinBox->changeValue(bounds.x());
        positionYSpinBox->changeValue(bounds.y());
    }
}

void DefaultToolWidget::slotRepositionShapes()
{
    const bool useGlobalSize = chkGlobalCoordinates->isChecked();
    const KoFlake::AnchorPosition anchor = positionSelector->value();

    QList<KoShape*> shapes;
    KoSelection *selection = m_tool->canvas()->shapeManager()->selection();
    QRectF bounds = calculateSelectionBounds(selection, anchor, useGlobalSize, &shapes);

    if (bounds.isNull()) return;

    const QPointF oldPosition = bounds.topLeft();
    const QPointF newPosition(positionXSpinBox->value(), positionYSpinBox->value());
    const QPointF diff = newPosition - oldPosition;

    if (diff.manhattanLength() < 1e-6) return;

    QList<QPointF> oldPositions;
    QList<QPointF> newPositions;

    Q_FOREACH (KoShape *shape, shapes) {
        const QPointF oldShapePosition = shape->absolutePosition(anchor);

        oldPositions << shape->absolutePosition(anchor);
        newPositions << oldShapePosition + diff;
    }

    KUndo2Command *cmd = new KoShapeMoveCommand(shapes, oldPositions, newPositions, anchor);
    m_tool->canvas()->addCommand(cmd);
}


void DefaultToolWidget::slotResizeShapes()
{
    const bool useGlobalSize = chkGlobalCoordinates->isChecked();
    const KoFlake::AnchorPosition anchor = positionSelector->value();

    QList<KoShape*> shapes;
    KoSelection *selection = m_tool->canvas()->shapeManager()->selection();
    QRectF bounds = calculateSelectionBounds(selection, anchor, useGlobalSize, &shapes);

    if (bounds.isNull()) return;

    const QSizeF oldSize(bounds.size());
    const QSizeF newSize(widthSpinBox->value(), heightSpinBox->value());

    const qreal scaleX = newSize.width() / oldSize.width();
    const qreal scaleY = newSize.height() / oldSize.height();

    if (qAbs(scaleX - 1.0) < 1e-6 && qAbs(scaleY - 1.0) < 1e-6) return;

    const bool usePostScaling =
        shapes.size() > 1 || chkUniformScaling->isChecked();

    KUndo2Command *cmd = new KoShapeResizeCommand(shapes,
                                                  scaleX, scaleY,
                                                  bounds.topLeft(),
                                                  usePostScaling,
                                                  selection->transformation());
    m_tool->canvas()->addCommand(cmd);
}

void DefaultToolWidget::setUnit(const KoUnit &unit)
{
    positionXSpinBox->setUnit(unit);
    positionYSpinBox->setUnit(unit);
    widthSpinBox->setUnit(unit);
    heightSpinBox->setUnit(unit);

    slotUpdatePositionBoxes();
    slotUpdateSizeBoxes();
}

void DefaultToolWidget::resourceChanged(int key, const QVariant &res)
{
    if (key == KoCanvasResourceManager::Unit) {
        setUnit(res.value<KoUnit>());
    } else if (key == DefaultTool::HotPosition) {
        positionSelector->setValue(KoFlake::AnchorPosition(res.toInt()));
    }
}
