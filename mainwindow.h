#pragma once
#include <QMainWindow>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QTabWidget>
#include <QGridLayout>
#include <vector>
#include "chartwidget.h"
#include "prng.h"

struct AnalysisResult {
    QString name;
    std::vector<int> freq;
    double expected;
    double avgDeviation;
    int count;
    int min_val;
    int max_val;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void runSingle();
    void runAll();
    void saveResults();
    void onTabChanged(int index);

private:
    void setupUi();
    void applyDarkStyle();
    AnalysisResult analyze(PRNG& gen, int count, int min_val, int max_val, int buckets);
    PRNG* createGenerator(int index, unsigned long long seed);
    void showResult(const AnalysisResult& r, ChartWidget* chart, QTextEdit* log);
    QString formatResult(const AnalysisResult& r);

    // Параметры
    QSpinBox*      spinSeed;
    QSpinBox*      spinCount;
    QSpinBox*      spinMin;
    QSpinBox*      spinMax;
    QSpinBox*      spinBuckets;
    QComboBox*     comboMethod;
    QPushButton*   btnRun;
    QPushButton*   btnRunAll;
    QPushButton*   btnSave;

    // Вкладки результатов
    QTabWidget*    tabResults;

    // Одиночный результат
    ChartWidget*   singleChart;
    QTextEdit*     singleLog;

    // Сравнение всех
    std::vector<ChartWidget*> allCharts;
    QTextEdit*     allLog;

    std::vector<AnalysisResult> lastResults;
};
