#include "mainwindow.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <cmath>
#include <algorithm>
#include <memory>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    applyDarkStyle();
    setWindowTitle("Генератор псевдослучайных чисел");
    setMinimumSize(900, 640);
    resize(1100, 720);
}

MainWindow::~MainWindow() {}

// ============================================================
// СОЗДАНИЕ ГЕНЕРАТОРА ПО ИНДЕКСУ
// ============================================================
PRNG* MainWindow::createGenerator(int index, unsigned long long seed) {
    switch (index) {
    case 0: return new MiddleSquare(seed);
    case 1: return new LCG(seed);
    case 2: return new Multiplicative(seed);
    case 3: return new XORshift(seed);
    case 4: return new PCG(seed);
    case 5: return new AdditiveLF(seed);
    }
    return nullptr;
}

// ============================================================
// АНАЛИЗ РАСПРЕДЕЛЕНИЯ
// ============================================================
AnalysisResult MainWindow::analyze(PRNG& gen, int count, int min_val, int max_val, int buckets) {
    AnalysisResult r;
    r.name    = gen.name();
    r.count   = count;
    r.min_val = min_val;
    r.max_val = max_val;
    r.freq.assign(buckets, 0);
    r.expected = static_cast<double>(count) / buckets;

    int range = max_val - min_val + 1;
    for (int i = 0; i < count; ++i) {
        int num = gen.generate(min_val, max_val);
        int idx = (num - min_val) * buckets / range;
        if (idx >= buckets) idx = buckets - 1;
        r.freq[idx]++;
    }

    double totalDev = 0.0;
    for (int f : r.freq)
        totalDev += std::fabs(f - r.expected) / r.expected * 100.0;
    r.avgDeviation = totalDev / buckets;

    return r;
}

// ============================================================
// ФОРМАТИРОВАНИЕ РЕЗУЛЬТАТА В ТЕКСТ
// ============================================================
QString MainWindow::formatResult(const AnalysisResult& r) {
    QString out;
    out += "══════════════════════════════════════════════\n";
    out += "  " + r.name + "\n";
    out += "══════════════════════════════════════════════\n";
    out += QString("Чисел: %1  |  Диапазон: [%2, %3]  |  Интервалов: %4\n")
               .arg(r.count).arg(r.min_val).arg(r.max_val).arg(r.freq.size());
    out += QString("Ожидаемое в интервале: %1\n\n").arg(r.expected, 0, 'f', 1);
    out += "  №  | Частота | Отклонение\n";
    out += " ----+---------+-----------\n";

    for (int i = 0; i < static_cast<int>(r.freq.size()); i++) {
        double dev = std::fabs(r.freq[i] - r.expected) / r.expected * 100.0;
        out += QString("  %1  |  %2     | %3%\n")
                   .arg(i + 1, 2)
                   .arg(r.freq[i], 5)
                   .arg(dev, 5, 'f', 1);
    }

    out += " ----+---------+-----------\n";
    out += QString("Среднее отклонение: %1%\n").arg(r.avgDeviation, 0, 'f', 2);

    QString q;
    if (r.avgDeviation < 3.0)       q = "✓ Отличное";
    else if (r.avgDeviation < 7.0)  q = "✓ Хорошее";
    else if (r.avgDeviation < 15.0) q = "△ Удовлетворительное";
    else                            q = "✗ Плохое";
    out += "Качество: " + q + "\n\n";

    return out;
}

// ============================================================
// ПОКАЗ РЕЗУЛЬТАТА НА ВКЛАДКЕ
// ============================================================
void MainWindow::showResult(const AnalysisResult& r, ChartWidget* chart, QTextEdit* log) {
    ChartData cd;
    cd.generatorName = r.name;
    cd.freq          = r.freq;
    cd.expected      = r.expected;
    cd.avgDeviation  = r.avgDeviation;
    chart->setData(cd);
    log->append(formatResult(r));
}

// ============================================================
// ЗАПУСК ОДНОГО ГЕНЕРАТОРА
// ============================================================
void MainWindow::runSingle() {
    unsigned long long seed    = static_cast<unsigned long long>(spinSeed->value());
    int count   = spinCount->value();
    int min_val = spinMin->value();
    int max_val = spinMax->value();
    int buckets = spinBuckets->value();

    if (min_val >= max_val) {
        QMessageBox::warning(this, "Ошибка", "min должен быть меньше max!");
        return;
    }

    std::unique_ptr<PRNG> gen(createGenerator(comboMethod->currentIndex(), seed));
    if (!gen) return;

    singleLog->clear();
    AnalysisResult r = analyze(*gen, count, min_val, max_val, buckets);
    showResult(r, singleChart, singleLog);
    tabResults->setCurrentIndex(0);
}

// ============================================================
// ЗАПУСК ВСЕХ ГЕНЕРАТОРОВ
// ============================================================
void MainWindow::runAll() {
    unsigned long long seed    = static_cast<unsigned long long>(spinSeed->value());
    int count   = spinCount->value();
    int min_val = spinMin->value();
    int max_val = spinMax->value();
    int buckets = spinBuckets->value();

    if (min_val >= max_val) {
        QMessageBox::warning(this, "Ошибка", "min должен быть меньше max!");
        return;
    }

    allLog->clear();
    lastResults.clear();

    for (int i = 0; i < 6; i++) {
        std::unique_ptr<PRNG> gen(createGenerator(i, seed));
        if (!gen) continue;
        AnalysisResult r = analyze(*gen, count, min_val, max_val, buckets);
        lastResults.push_back(r);
        showResult(r, allCharts[i], allLog);
    }

    // Итоговая таблица сравнения
    allLog->append("══════════════════════════════════════════════\n");
    allLog->append("  СРАВНЕНИЕ ГЕНЕРАТОРОВ\n");
    allLog->append("══════════════════════════════════════════════\n");

    std::vector<AnalysisResult> sorted = lastResults;
    std::sort(sorted.begin(), sorted.end(),
              [](const AnalysisResult& a, const AnalysisResult& b){
                  return a.avgDeviation < b.avgDeviation;
              });

    for (int i = 0; i < static_cast<int>(sorted.size()); i++) {
        allLog->append(QString("  %1. %2 — %3%")
                           .arg(i + 1)
                           .arg(sorted[i].name)
                           .arg(sorted[i].avgDeviation, 0, 'f', 2));
    }
    allLog->append("");

    tabResults->setCurrentIndex(1);
}

// ============================================================
// СОХРАНЕНИЕ В ФАЙЛ
// ============================================================
void MainWindow::saveResults() {
    if (lastResults.empty() && tabResults->currentIndex() == 1) {
        QMessageBox::information(this, "Нет данных",
                                 "Сначала запустите генераторы.");
        return;
    }

    QString path = QFileDialog::getSaveFileName(
        this, "Сохранить результаты", "", "Текстовые файлы (*.txt)");
    if (path.isEmpty()) return;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл.");
        return;
    }

    QTextStream out(&f);
    if (tabResults->currentIndex() == 0)
        out << singleLog->toPlainText();
    else
        out << allLog->toPlainText();

    QMessageBox::information(this, "Сохранено", "Файл успешно сохранён.");
}

void MainWindow::onTabChanged(int) {}

// ============================================================
// ПОСТРОЕНИЕ UI
// ============================================================
void MainWindow::setupUi() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    // --- Панель параметров ---
    QGroupBox* gbParams = new QGroupBox("Параметры генерации");
    QGridLayout* gl = new QGridLayout(gbParams);
    gl->setSpacing(8);

    auto addRow = [&](int row, const QString& label, QWidget* w) {
        gl->addWidget(new QLabel(label), row, 0);
        gl->addWidget(w, row, 1);
    };

    spinSeed    = new QSpinBox;   spinSeed->setRange(1, 999999999); spinSeed->setValue(12345);
    spinCount   = new QSpinBox;   spinCount->setRange(10, 1000000); spinCount->setValue(10000);
                                  spinCount->setSingleStep(1000);
    spinMin     = new QSpinBox;   spinMin->setRange(-999999, 999999); spinMin->setValue(1);
    spinMax     = new QSpinBox;   spinMax->setRange(-999999, 999999); spinMax->setValue(100);
    spinBuckets = new QSpinBox;   spinBuckets->setRange(2, 50); spinBuckets->setValue(10);

    comboMethod = new QComboBox;
    comboMethod->addItems({
        "1. Метод середины квадрата",
        "2. LCG (линейный конгруэнтный)",
        "3. Мультипликативный",
        "4. XORshift",
        "5. PCG (современный)",
        "6. Аддитивный (Lagged Fibonacci)"
    });

    addRow(0, "Seed:",            spinSeed);
    addRow(1, "Количество:",      spinCount);
    addRow(2, "Минимум:",         spinMin);
    addRow(3, "Максимум:",        spinMax);
    addRow(4, "Интервалов:",      spinBuckets);
    addRow(5, "Метод:",           comboMethod);

    btnRun    = new QPushButton("Запустить");
    btnRunAll = new QPushButton("Сравнить все");
    btnSave   = new QPushButton("Сохранить");

    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addWidget(btnRun);
    btnLayout->addWidget(btnRunAll);
    btnLayout->addWidget(btnSave);
    gl->addLayout(btnLayout, 6, 0, 1, 2);

    connect(btnRun,    &QPushButton::clicked, this, &MainWindow::runSingle);
    connect(btnRunAll, &QPushButton::clicked, this, &MainWindow::runAll);
    connect(btnSave,   &QPushButton::clicked, this, &MainWindow::saveResults);

    // --- Вкладки результатов ---
    tabResults = new QTabWidget;
    connect(tabResults, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    // Вкладка 1: одиночный результат
    QWidget* singleTab = new QWidget;
    QSplitter* singleSplit = new QSplitter(Qt::Vertical, singleTab);
    singleChart = new ChartWidget;
    singleChart->setMinimumHeight(240);
    singleLog = new QTextEdit;
    singleLog->setReadOnly(true);
    singleLog->setFont(QFont("Consolas", 9));
    singleLog->setMinimumHeight(140);
    singleSplit->addWidget(singleChart);
    singleSplit->addWidget(singleLog);
    singleSplit->setStretchFactor(0, 2);
    singleSplit->setStretchFactor(1, 1);
    QVBoxLayout* sv = new QVBoxLayout(singleTab);
    sv->setContentsMargins(0, 0, 0, 0);
    sv->addWidget(singleSplit);
    tabResults->addTab(singleTab, "Один генератор");

    // Вкладка 2: все генераторы
    QWidget* allTab = new QWidget;
    QSplitter* allSplit = new QSplitter(Qt::Vertical, allTab);

    QWidget* chartsContainer = new QWidget;
    QGridLayout* chartsGrid = new QGridLayout(chartsContainer);
    chartsGrid->setSpacing(6);
    chartsGrid->setContentsMargins(4, 4, 4, 4);

    allCharts.resize(6);
    for (int i = 0; i < 6; i++) {
        allCharts[i] = new ChartWidget;
        allCharts[i]->setMinimumHeight(200);
        chartsGrid->addWidget(allCharts[i], i / 3, i % 3);
    }

    QScrollArea* scrollCharts = new QScrollArea;
    scrollCharts->setWidget(chartsContainer);
    scrollCharts->setWidgetResizable(true);
    scrollCharts->setMinimumHeight(200);

    allLog = new QTextEdit;
    allLog->setReadOnly(true);
    allLog->setFont(QFont("Consolas", 9));
    allLog->setMinimumHeight(120);

    allSplit->addWidget(scrollCharts);
    allSplit->addWidget(allLog);
    allSplit->setStretchFactor(0, 3);
    allSplit->setStretchFactor(1, 1);

    QVBoxLayout* av = new QVBoxLayout(allTab);
    av->setContentsMargins(0, 0, 0, 0);
    av->addWidget(allSplit);
    tabResults->addTab(allTab, "Сравнение всех (6)");

    // --- Главный сплиттер ---
    QSplitter* mainSplit = new QSplitter(Qt::Horizontal, central);
    mainSplit->addWidget(gbParams);
    mainSplit->addWidget(tabResults);
    mainSplit->setStretchFactor(0, 0);
    mainSplit->setStretchFactor(1, 1);
    mainSplit->setSizes({280, 800});

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->addWidget(mainSplit);
}

// ============================================================
// ТЁМНАЯ ТЕМА
// ============================================================
void MainWindow::applyDarkStyle() {
    qApp->setStyle("Fusion");
    qApp->setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #1e1e2e;
            color: #cdd6f4;
            font-family: "Segoe UI";
            font-size: 11px;
        }
        QGroupBox {
            border: 1px solid #45475a;
            border-radius: 6px;
            margin-top: 14px;
            padding: 8px;
            font-weight: bold;
            color: #cba6f7;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
        }
        QSpinBox, QComboBox {
            background: #313244;
            border: 1px solid #45475a;
            border-radius: 4px;
            padding: 4px 8px;
            color: #cdd6f4;
        }
        QSpinBox:focus, QComboBox:focus {
            border-color: #89b4fa;
        }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView {
            background: #313244;
            color: #cdd6f4;
            selection-background-color: #45475a;
        }
        QPushButton {
            background: #313244;
            border: 1px solid #45475a;
            border-radius: 5px;
            padding: 7px 16px;
            color: #cdd6f4;
            font-weight: bold;
        }
        QPushButton:hover  { background: #45475a; border-color: #89b4fa; }
        QPushButton:pressed { background: #585b70; }
        QTextEdit {
            background: #181825;
            border: 1px solid #313244;
            border-radius: 4px;
            color: #a6e3a1;
            font-family: Consolas;
        }
        QTabWidget::pane {
            border: 1px solid #45475a;
            background: #1e1e2e;
        }
        QTabBar::tab {
            background: #313244;
            color: #a6adc8;
            padding: 7px 20px;
            border-radius: 4px;
            margin-right: 2px;
        }
        QTabBar::tab:selected { background: #45475a; color: #cdd6f4; }
        QTabBar::tab:hover    { background: #3d3f56; }
        QSplitter::handle {
            background: #45475a;
            width: 2px; height: 2px;
        }
        QScrollBar:vertical {
            background: #1e1e2e; width: 10px;
        }
        QScrollBar::handle:vertical {
            background: #45475a; border-radius: 4px;
        }
        QLabel { color: #bac2de; }
    )");
}
