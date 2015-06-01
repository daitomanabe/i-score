#pragma once
#include <QGraphicsItem>
class CurveSegmentModel;
class CurveSegmentView : public QGraphicsObject
{
        Q_OBJECT
    public:
        CurveSegmentView(CurveSegmentModel* model, QGraphicsItem* parent);

        void setRect(const QRectF& theRect);
        void setPoints(QVector<QPointF>&& thePoints);

        QRectF boundingRect() const override;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    signals:
        void pressed(const QPointF&);
        void moved(const QPointF&);
        void released(const QPointF&);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    private:
        // Takes a table of points and draws them in a square given by the boundingRect
        // QGraphicsItem interface
        QVector<QPointF> points; // each between rect.topLeft() :: rect.bottomRight()
        QRectF rect;

        CurveSegmentModel* m_model{};
};