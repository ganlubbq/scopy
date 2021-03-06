/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "plot_line_handle.h"
#include "handles_area.hpp"

#include <QPainter>
#include <QMoveEvent>

#include <QDebug>

PlotLineHandle::PlotLineHandle(const QPixmap &handleIcon, QWidget *parent):
	QWidget(parent),
	m_enable_silent_move(false),
	m_innerSpacing(0),
	m_outerSpacing(0),
	m_image(handleIcon),
	m_grabbed(false),
	m_current_pos(0)
{
}

void PlotLineHandle::moveSilently(QPoint pos)
{
	m_enable_silent_move = true;
	moveWithinParent(pos.x(), pos.y());
}

int PlotLineHandle::position()
{
	return m_current_pos;
}

void PlotLineHandle::setPen(const QPen& pen)
{
	m_pen = pen;
}

const QPen& PlotLineHandle::pen()
{
	return m_pen;
}

void PlotLineHandle::mousePressEvent(QMouseEvent *event)
{
	QWidget *parent = static_cast<QWidget *>(this->parent());

	parent->raise();

	QWidget::mousePressEvent(event);
}

void PlotLineHandle::mouseReleaseEvent(QMouseEvent *event)
{
	Q_EMIT mouseReleased();

	QWidget::mouseReleaseEvent(event);
}

void PlotLineHandle::setGrabbed(bool grabbed)
{
	if (m_grabbed != grabbed) {
		m_grabbed = grabbed;
		Q_EMIT grabbedChanged(grabbed);
	}
}

PlotLineHandleH::PlotLineHandleH(const QPixmap &handleIcon, QWidget *parent,
			bool facingBottom):
	PlotLineHandle(handleIcon, parent),
	m_facingBottom(facingBottom)
{
	m_innerSpacing = m_image.width() / 2;
	m_width = m_image.width();
	m_height = m_innerSpacing + m_image.height() + m_outerSpacing;
	setMinimumSize(m_width, m_height);
	setMaximumSize(m_width, m_height);
}

void PlotLineHandleH::triggerMove()
{
	Q_EMIT positionChanged(m_current_pos);
}

void PlotLineHandleH::setPosition(int pos)
{
	if (m_current_pos != pos)
		moveWithinParent(centerPosToOrigin(pos), 0);
}

void PlotLineHandleH::setPositionSilenty(int pos)
{
	if (m_current_pos != pos)
		moveSilently(QPoint(centerPosToOrigin(pos), 0));
}

void PlotLineHandleH::setInnerSpacing(int value)
{
	m_innerSpacing = value;
}

void PlotLineHandleH::moveWithinParent(int x, int y)
{
	Q_UNUSED(y);

	HorizHandlesArea *area = static_cast<HorizHandlesArea *>(parent());

	int lower_limit = 0 + area->leftPadding() - width() / 2;
	int upper_limit = area->width() - area->rightPadding() - width() / 2 - 1;

	if (x < lower_limit)
		x = lower_limit;
	else if (x > upper_limit)
		x = upper_limit;

	int centerPos = originPosToCenter(x);
	int oldCenterPos = originPosToCenter(this->x());

	if (centerPos != oldCenterPos) {
		move(x, this->y());

		if (!m_enable_silent_move)
			Q_EMIT positionChanged(centerPos);
	}
	m_enable_silent_move = false;
	m_current_pos = centerPos;
}

void PlotLineHandleH::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QPoint lineStart;
	QPoint imageTopLeft;

	p.setPen(m_pen);

	if (m_facingBottom) {
		lineStart = QPoint(m_image.width() / 2,
				m_image.height());
		imageTopLeft = QPoint(0, 0);
	} else {
		lineStart = QPoint(m_image.width() / 2, 0);
		imageTopLeft = QPoint(0, m_innerSpacing);
	}

	p.drawLine(lineStart, lineStart + QPoint(0, m_innerSpacing));
	p.drawPixmap(imageTopLeft, m_image);
}

int PlotLineHandleH::originPosToCenter(int origin)
{
	HorizHandlesArea *area = static_cast<HorizHandlesArea *>(parent());
	int offset = -area->leftPadding() + width() / 2;

	return (origin + offset);
}

int PlotLineHandleH::centerPosToOrigin(int center)
{
	HorizHandlesArea *area = static_cast<HorizHandlesArea *>(parent());
	int offset = -area->leftPadding() + width() / 2;

	return (center - offset);
}

PlotLineHandleV::PlotLineHandleV(const QPixmap &handleIcon, QWidget *parent,
			bool facingRight):
	PlotLineHandle(handleIcon, parent),
	m_facingRight(facingRight)
{
	m_innerSpacing = m_image.height() / 2;
	m_width = m_innerSpacing +  m_image.width()  + m_outerSpacing;
	m_height = m_image.height();
	setMinimumSize(m_width, m_height);
	setMaximumSize(m_width, m_height);
}

void PlotLineHandleV::triggerMove()
{
	Q_EMIT positionChanged(m_current_pos);
}

void PlotLineHandleV::setPosition(int pos)
{
	if (m_current_pos != pos)
		moveWithinParent(0, centerPosToOrigin(pos));
}

void PlotLineHandleV::setPositionSilenty(int pos)
{
	if (m_current_pos != pos)
		moveSilently(QPoint(0, centerPosToOrigin(pos)));
}

void PlotLineHandleV::moveWithinParent(int x, int y)
{
	Q_UNUSED(x);

	VertHandlesArea *area = static_cast<VertHandlesArea *>(parent());

	int lower_limit = 0 + area->topPadding() - height() / 2;
	int upper_limit = area->height() - area->bottomPadding() - height() / 2 - 1;

	if (y < lower_limit)
		y = lower_limit;
	else if (y > upper_limit)
		y = upper_limit;

	int centerPos = originPosToCenter(y);
	int oldCenterPos = originPosToCenter(this->y());

	if (centerPos != oldCenterPos) {
		move(this->x(), y);
		if (!m_enable_silent_move)
			Q_EMIT positionChanged(centerPos);
	}
	m_enable_silent_move = false;
	m_current_pos = centerPos;
}

void PlotLineHandleV::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QPoint lineStart;
	QPoint imageTopLeft;

	p.setPen(m_pen);

	if (m_facingRight) {
		lineStart = QPoint(m_image.width(), m_image.height() / 2);
		imageTopLeft = QPoint(0, 0);
	} else {
		lineStart = QPoint(0, m_image.height() / 2);
		imageTopLeft = QPoint(m_innerSpacing, 0);
	}

	p.drawLine(lineStart, lineStart + QPoint(m_innerSpacing, 0));
	p.drawPixmap(imageTopLeft, m_image);
}

int PlotLineHandleV::originPosToCenter(int origin)
{
	VertHandlesArea *area = static_cast<VertHandlesArea *>(parent());
	int offset = -area->topPadding() + height() / 2;

	return (origin + offset);
}

int PlotLineHandleV::centerPosToOrigin(int center)
{
	VertHandlesArea *area = static_cast<VertHandlesArea *>(parent());
	int offset = -area->topPadding() + height() / 2;

	return (center - offset);
}

FreePlotLineHandleH::FreePlotLineHandleH(const QPixmap &handleIcon,
			const QPixmap &beyondLeftIcon,
			const QPixmap &beyondRightIcon,
			QWidget *parent, bool facingRight):
		PlotLineHandleH(handleIcon, parent, facingRight),
		m_beyondLeftImage(beyondLeftIcon),
		m_beyondRightImage(beyondRightIcon),
		m_isBeyondLeft(false),
		m_isBeyondRight(false)
{
}

void FreePlotLineHandleH::moveWithinParent(int x, int y)
{
	Q_UNUSED(y);

	HorizHandlesArea *area = static_cast<HorizHandlesArea *>(parent());

	int lower_limit = 0 + area->leftPadding() - width() / 2;
	int upper_limit = area->width() - area->rightPadding() - width() / 2 - 1;

	int centerPos = originPosToCenter(x);
	int oldCenterPos = m_current_pos;

	if (centerPos != oldCenterPos) {
		m_isBeyondLeft = false;
		m_isBeyondRight = false;
		if (x < lower_limit) {
			x = lower_limit;
			m_isBeyondLeft = true;
		} else if (x > upper_limit) {
			x = upper_limit;
			m_isBeyondRight = true;
		}
		move(x, this->y());

		if (!m_enable_silent_move) {
			Q_EMIT positionChanged(centerPos);
			this->update();
		}
		m_current_pos = centerPos;
	}
	m_enable_silent_move = false;
}

void FreePlotLineHandleH::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QPoint lineStart;
	QPoint imageTopLeft;

	p.setPen(m_pen);

	if (m_facingBottom) {
		lineStart = QPoint(m_image.width() / 2,
				m_image.height());
		imageTopLeft = QPoint(0, 0);
	} else {
		lineStart = QPoint(m_image.width() / 2, 0);
		imageTopLeft = QPoint(0, m_innerSpacing);
	}

	if (m_isBeyondLeft) {
		p.drawPixmap(imageTopLeft, m_beyondLeftImage);
	} else if (m_isBeyondRight) {
		p.drawPixmap(imageTopLeft, m_beyondRightImage);
	} else {
		p.drawLine(lineStart, lineStart + QPoint(0, m_innerSpacing));
		p.drawPixmap(imageTopLeft, m_image);
	}
}

FreePlotLineHandleV::FreePlotLineHandleV(const QPixmap &handleIcon,
			const QPixmap &beyondTopIcon,
			const QPixmap &beyondBottomIcon,
			QWidget *parent, bool facingRight):
		PlotLineHandleV(handleIcon, parent, facingRight),
		m_beyondTopImage(beyondTopIcon),
		m_beyondBottomImage(beyondBottomIcon),
		m_isBeyondTop(false),
		m_isBeyondBottom(false)
{
}

void FreePlotLineHandleV::moveWithinParent(int x, int y)
{
	Q_UNUSED(x);

	VertHandlesArea *area = static_cast<VertHandlesArea *>(parent());

	int lower_limit = 0 + area->topPadding() - height() / 2;
	int upper_limit = area->height() - area->bottomPadding() - height() / 2 - 1;

	int centerPos = originPosToCenter(y);
	int oldCenterPos = m_current_pos;

	if (centerPos != oldCenterPos) {
		m_isBeyondTop = false;
		m_isBeyondBottom = false;
		if (y < lower_limit) {
			y = lower_limit;
			m_isBeyondTop = true;
		} else if (y > upper_limit) {
			y = upper_limit;
			m_isBeyondBottom = true;
		}
		move(this->x(), y);

		if (!m_enable_silent_move) {
			Q_EMIT positionChanged(centerPos);
			this->update();
		}
		m_current_pos = centerPos;
	}
	m_enable_silent_move = false;
}

void FreePlotLineHandleV::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QPoint lineStart;
	QPoint imageTopLeft;

	p.setPen(m_pen);

	if (m_facingRight) {
		lineStart = QPoint(m_image.width(), m_image.height() / 2);
		imageTopLeft = QPoint(0, 0);
	} else {
		lineStart = QPoint(0, m_image.height() / 2);
		imageTopLeft = QPoint(m_innerSpacing, 0);
	}

	if (m_isBeyondTop) {
		p.drawPixmap(imageTopLeft, m_beyondTopImage);
	} else if (m_isBeyondBottom) {
		p.drawPixmap(imageTopLeft, m_beyondBottomImage);
	} else {
		p.drawLine(lineStart, lineStart + QPoint(m_innerSpacing, 0));
		p.drawPixmap(imageTopLeft, m_image);
	}
}

RoundedHandleV::RoundedHandleV(const QPixmap &handleIcon,
			const QPixmap &beyondTopIcon,
			const QPixmap &beyondBottomIcon,
			QWidget *parent, bool facingRight):
		FreePlotLineHandleV(handleIcon, beyondTopIcon, beyondBottomIcon,
			 parent, facingRight)
{
	m_innerSpacing = m_image.height();
	m_width = m_innerSpacing +  m_image.width()  + m_outerSpacing;
	m_height = m_image.height();
	setMinimumSize(m_width, m_height);
	setMaximumSize(m_width, m_height);
}

QColor RoundedHandleV::roundRectColor()
{
	return m_roundRectColor;
}

void RoundedHandleV::setRoundRectColor(const QColor &newColor)
{
	if (m_roundRectColor != newColor) {
		m_roundRectColor = newColor;
		this->update();
	}
}

void RoundedHandleV::paintEvent(QPaintEvent *pv)
{
	QPainter p(this);
	QRect rect(0, 0, m_image.width() - 1, m_image.height() - 1);

	p.setPen(QPen(m_roundRectColor, 1, Qt::SolidLine));
	p.setBrush(m_roundRectColor);
	p.setRenderHint(QPainter::Antialiasing);
	p.drawRoundedRect(rect, 30, 30, Qt::RelativeSize);

	FreePlotLineHandleV::paintEvent(pv);
}
