/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qtest.h>

#include <QtGui>

#include "benchmarktests.h"

//TESTED_FILES=

class BenchWidget : public QWidget
{
public:
    BenchWidget(Benchmark *benchmark);

    void paintEvent(QPaintEvent *event);

    bool done() const { return m_done; }
    qreal result() const { return m_result; }

public:
    QTime timer;

    Benchmark *m_benchmark;

    bool m_done;
    qreal m_result;

    uint m_total;
    uint m_iteration;

    QVector<uint> iterationTimes;
};

void BenchWidget::paintEvent(QPaintEvent *)
{
    if (m_done)
        return;

    QPainter p(this);

    m_benchmark->begin(&p, 100);

    PaintingRectAdjuster adjuster;
    adjuster.setNewBenchmark(m_benchmark);
    adjuster.reset(rect());

    for (int i = 0; i < 100; ++i)
        m_benchmark->draw(&p, adjuster.newPaintingRect(), i);

    m_benchmark->end(&p);

    ++m_iteration;

    uint currentElapsed = timer.isNull() ? 0 : timer.elapsed();
    timer.restart();

    m_total += currentElapsed;

    // warm up for at most 5 iterations or half a second
    if (m_iteration >= 5 || m_total >= 500) {
        iterationTimes << currentElapsed;

        if (iterationTimes.size() >= 5) {
            qreal mean = 0;
            qreal stddev = 0;
            uint min = INT_MAX;

            for (int i = 0; i < iterationTimes.size(); ++i) {
                mean += iterationTimes.at(i);
                min = qMin(min, iterationTimes.at(i));
            }

            mean /= qreal(iterationTimes.size());

            for (int i = 0; i < iterationTimes.size(); ++i) {
                qreal delta = iterationTimes.at(i) - mean;
                stddev += delta * delta;
            }

            stddev = qSqrt(stddev / iterationTimes.size());

            stddev = 100 * stddev / mean;
            // do 50 iterations, break earlier if we spend more than 5 seconds or have a low std deviation after 2 seconds
            if (iterationTimes.size() >= 50 || m_total >= 5000 || (m_total >= 2000 && stddev < 4)) {
                m_result = min;
                m_done = true;
                return;
            }
        }
    }
}

BenchWidget::BenchWidget(Benchmark *benchmark)
    : m_benchmark(benchmark)
    , m_done(false)
    , m_result(0)
    , m_total(0)
    , m_iteration(0)
{
    setWindowTitle(benchmark->name());
    resize(640, 480);
}

class tst_QtBench : public QObject
{
    Q_OBJECT

private slots:
    void qtBench();
    void qtBench_data();
};

QString makeString(int length)
{
    const char chars[] = "abcd efgh ijkl mnop qrst uvwx yz!$. ABCD 1234";
    int len = strlen(chars);

    QString ret;
    for (int j = 0; j < length; j++) {
        ret += QChar(chars[(j * 97) % len]);
    }

    return ret;
}

void tst_QtBench::qtBench_data()
{
    QTest::addColumn<void *>("benchmark");

    QString shortString = makeString(5);
    QString middleString = makeString(50);
    QString longString = makeString(35) + "\n"
                         + makeString(45) + "\n"
                         + makeString(75);
    QString superLongString = "Lorem ipsum dolor sit am\n"
                              "et, consectetur adipisci\n"
                              "ng elit. Integer mi leo,\n"
                              "interdum ut congue at, p\n"
                              "ulvinar et tellus. Quisq\n"
                              "ue pretium eleifend laci\n"
                              "nia. Ut semper gravida l\n"
                              "ectus in commodo. Vestib\n"
                              "ulum pharetra arcu in en\n"
                              "im ultrices hendrerit. P\n"
                              "ellentesque habitant mor\n"
                              "bi tristique senectus et\n"
                              "netus et malesuada fames\n"
                              "ac turpis egestas. Ut er\n"
                              "os sem, feugiat in eleme\n"
                              "ntum in, porta sit amet \n"
                              "neque. Fusce mi tellus, \n"
                              "congue non dapibus eget,\n"
                              "pharetra quis quam. Duis\n"
                              "dui massa, pulvinar ac s\n"
                              "odales pharetra, dictum \n"
                              "in enim. Phasellus a nis\n"
                              "i erat, sed pellentesque\n"
                              "mi. Curabitur sed.";

    QList<Benchmark *> benchmarks;
    benchmarks << (new DrawText(shortString, DrawText::PainterMode));
    benchmarks << (new DrawText(middleString, DrawText::PainterMode));
    benchmarks << (new DrawText(longString, DrawText::PainterMode));
    benchmarks << (new DrawText(superLongString, DrawText::PainterMode));

    benchmarks << (new DrawText(shortString, DrawText::PainterQPointMode));
    benchmarks << (new DrawText(middleString, DrawText::PainterQPointMode));
    benchmarks << (new DrawText(longString, DrawText::PainterQPointMode));
    benchmarks << (new DrawText(superLongString, DrawText::PainterQPointMode));

    benchmarks << (new DrawText(shortString, DrawText::PixmapMode));
    benchmarks << (new DrawText(middleString, DrawText::PixmapMode));
    benchmarks << (new DrawText(longString, DrawText::PixmapMode));
    benchmarks << (new DrawText(superLongString, DrawText::PixmapMode));

    benchmarks << (new DrawText(shortString, DrawText::StaticTextMode));
    benchmarks << (new DrawText(middleString, DrawText::StaticTextMode));
    benchmarks << (new DrawText(longString, DrawText::StaticTextMode));
    benchmarks << (new DrawText(superLongString, DrawText::StaticTextMode));

    benchmarks << (new DrawText(shortString, DrawText::StaticTextWithMaximumSizeMode));
    benchmarks << (new DrawText(middleString, DrawText::StaticTextWithMaximumSizeMode));
    benchmarks << (new DrawText(longString, DrawText::StaticTextWithMaximumSizeMode));
    benchmarks << (new DrawText(superLongString, DrawText::StaticTextWithMaximumSizeMode));

    benchmarks << (new DrawText(shortString, DrawText::StaticTextBackendOptimizations));
    benchmarks << (new DrawText(middleString, DrawText::StaticTextBackendOptimizations));
    benchmarks << (new DrawText(longString, DrawText::StaticTextBackendOptimizations));
    benchmarks << (new DrawText(superLongString, DrawText::StaticTextBackendOptimizations));

    foreach (Benchmark *benchmark, benchmarks)
        QTest::newRow(qPrintable(benchmark->name())) << reinterpret_cast<void *>(benchmark);
}

void tst_QtBench::qtBench()
{
    QFETCH(void *, benchmark);

    BenchWidget widget(reinterpret_cast<Benchmark *>(benchmark));
    widget.show();
    QTest::qWaitForWindowShown(&widget);

    while (!widget.done()) {
        widget.update();
        QApplication::processEvents();
    }

    QTest::setBenchmarkResult(widget.result(), QTest::WalltimeMilliseconds);
}

QTEST_MAIN(tst_QtBench)
#include "tst_qtbench.moc"
