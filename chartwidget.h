#pragma once
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <vector>
#include <cmath>

struct ChartData {
    QString generatorName;
    std::vector<int> freq;
    double expected;
    double avgDeviation;
};

class ChartWidget : public QWidget {
public:
    explicit ChartWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(220);
        setMinimumWidth(300);
    }

    void setData(const ChartData& d) {
        data = d;
        update();
    }

    void clear() {
        data = {};
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QColor bg(30, 30, 46);
        p.fillRect(rect(), bg);

        if (data.freq.empty()) {
            p.setPen(QColor(150, 150, 170));
            p.setFont(QFont("Segoe UI", 11));
            p.drawText(rect(), Qt::AlignCenter, "Нет данных");
            return;
        }

        const int pad = 40;
        const int padTop = 30;
        const int padBottom = 35;
        int chartW = width() - pad * 2;
        int chartH = height() - padTop - padBottom;

        p.fillRect(pad, padTop, chartW, chartH, QColor(40, 40, 58));

        int n = static_cast<int>(data.freq.size());
        int maxFreq = *std::max_element(data.freq.begin(), data.freq.end());
        maxFreq = std::max(maxFreq, 1);

        double barW = static_cast<double>(chartW) / n;
        double gap = barW * 0.12;

        double expectedY = padTop + chartH - (data.expected / maxFreq) * chartH;
        p.setPen(QPen(QColor(120, 220, 120, 160), 1.5, Qt::DashLine));
        p.drawLine(QPointF(pad, expectedY), QPointF(pad + chartW, expectedY));

        for (int i = 0; i < n; i++) {
            double barH = (static_cast<double>(data.freq[i]) / maxFreq) * chartH;
            double x = pad + i * barW + gap;
            double y = padTop + chartH - barH;
            double w = barW - gap * 2;

            double dev = std::fabs(data.freq[i] - data.expected) / data.expected;
            QColor barColor;
            if (dev < 0.05)       barColor = QColor(80, 200, 140);
            else if (dev < 0.12)  barColor = QColor(90, 160, 230);
            else if (dev < 0.20)  barColor = QColor(240, 190, 60);
            else                  barColor = QColor(220, 80, 80);

            QLinearGradient grad(x, y, x, y + barH);
            grad.setColorAt(0, barColor.lighter(130));
            grad.setColorAt(1, barColor.darker(120));
            p.fillRect(QRectF(x, y, w, barH), grad);

            p.setPen(QColor(180, 180, 200));
            p.setFont(QFont("Segoe UI", 7));
            p.drawText(QRectF(x, padTop + chartH + 4, w, 14),
                       Qt::AlignHCenter, QString::number(i + 1));
        }

        p.setPen(QColor(70, 70, 100));
        p.drawRect(pad, padTop, chartW, chartH);

        p.setPen(QColor(120, 220, 120));
        p.setFont(QFont("Segoe UI", 8));
        p.drawText(pad + 4, static_cast<int>(expectedY) - 3, "ожидаемое");

        p.setPen(QColor(210, 210, 230));
        p.setFont(QFont("Segoe UI", 9, QFont::Bold));
        p.drawText(QRect(pad, 4, chartW, 22), Qt::AlignCenter, data.generatorName);

        QString quality;
        QColor qColor;
        if (data.avgDeviation < 3.0)       { quality = "Отличное"; qColor = QColor(80, 200, 140); }
        else if (data.avgDeviation < 7.0)  { quality = "Хорошее";  qColor = QColor(90, 160, 230); }
        else if (data.avgDeviation < 15.0) { quality = "Удовл.";   qColor = QColor(240, 190, 60); }
        else                               { quality = "Плохое";   qColor = QColor(220, 80, 80);  }

        p.setPen(qColor);
        p.setFont(QFont("Segoe UI", 8));
        p.drawText(QRect(0, padTop + chartH + 18, width(), 16), Qt::AlignCenter,
                   QString("Отклонение: %1% — %2")
                       .arg(data.avgDeviation, 0, 'f', 1)
                       .arg(quality));
    }

private:
    ChartData data;
};