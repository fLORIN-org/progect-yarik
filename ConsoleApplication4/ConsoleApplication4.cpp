#include <iostream>
#include <vector>
#include <cmath>
#include <locale>
#include <string>

using namespace std;

// ============================================================
// БАЗОВЫЙ КЛАСС ДЛЯ ГЕНЕРАТОРОВ
// ============================================================
class PRNG {
protected:
    unsigned long long seed;
public:
    PRNG(unsigned long long s) : seed(s) {}
    virtual ~PRNG() {}

    virtual int generate(int min_val, int max_val) = 0;
    virtual string get_name() const = 0;
};

// ============================================================
// 1. МЕТОД СЕРЕДИНЫ КВАДРАТА
// ============================================================
class MiddleSquare : public PRNG {
public:
    MiddleSquare(unsigned long long s) : PRNG(s) {}

    int generate(int min_val, int max_val) override {
        seed = seed * seed;
        seed = (seed / 100) % 10000;

        if (seed == 0) seed = 1234;

        return min_val + static_cast<int>(seed % (max_val - min_val + 1));
    }

    string get_name() const override {
        return "Метод середины квадрата";
    }
};

// ============================================================
// 2. ЛИНЕЙНЫЙ КОНГРУЭНТНЫЙ МЕТОД (LCG)
// ============================================================
class LCG : public PRNG {
public:
    LCG(unsigned long long s) : PRNG(s) {}

    int generate(int min_val, int max_val) override {
        const unsigned long long a = 1103515245;
        const unsigned long long c = 12345;
        const unsigned long long m = 1ULL << 31;

        seed = (a * seed + c) % m;

        return min_val + static_cast<int>(seed % (max_val - min_val + 1));
    }

    string get_name() const override {
        return "Линейный конгруэнтный метод (LCG)";
    }
};

// ============================================================
// 3. МУЛЬТИПЛИКАТИВНЫЙ МЕТОД
// ============================================================
class Multiplicative : public PRNG {
public:
    Multiplicative(unsigned long long s) : PRNG(s) {}

    int generate(int min_val, int max_val) override {
        const unsigned long long a = 48271;
        const unsigned long long m = 2147483647;

        seed = (a * seed) % m;

        return min_val + static_cast<int>(seed % (max_val - min_val + 1));
    }

    string get_name() const override {
        return "Мультипликативный метод";
    }
};

// ============================================================
// 4. XORSHIFT
// ============================================================
class XORshift : public PRNG {
public:
    XORshift(unsigned long long s) : PRNG(s) {
        if (seed == 0) seed = 1;
    }

    int generate(int min_val, int max_val) override {
        seed ^= seed << 13;
        seed ^= seed >> 7;
        seed ^= seed << 17;

        unsigned int result = static_cast<unsigned int>(seed);
        return min_val + static_cast<int>(result % (max_val - min_val + 1));
    }

    string get_name() const override {
        return "XORshift (побитовый метод)";
    }
};
// ============================================================
// 5. PCG (Permuted Congruential Generator)
// ============================================================
class PCG : public PRNG {
private:
    unsigned long long state;
    static const unsigned long long multiplier = 6364136223846793005ULL;
    static const unsigned long long increment = 1442695040888963407ULL;

public:
    PCG(unsigned long long s) : PRNG(s) {
        state = s;
        state = (state * multiplier + increment) & 0xFFFFFFFFFFFFFFFFULL;
        state = (state * multiplier + increment) & 0xFFFFFFFFFFFFFFFFULL;
    }

    int generate(int min_val, int max_val) override {
        unsigned long long oldstate = state;

        state = (oldstate * multiplier + increment) & 0xFFFFFFFFFFFFFFFFULL;

        unsigned int xorshifted = static_cast<unsigned int>(((oldstate >> 18) ^ oldstate) >> 27);
        unsigned int rot = static_cast<unsigned int>(oldstate >> 59);

        unsigned int result = (xorshifted >> rot) | (xorshifted << ((32 - rot) & 31));

        return min_val + static_cast<int>(result % (max_val - min_val + 1));
    }

    string get_name() const override {
        return "PCG (современный генератор)";
    }
};

// ============================================================
// 6. АДДИТИВНЫЙ ГЕНЕРАТОР (Lagged Fibonacci)
// ============================================================
class AdditiveLaggedFibonacci : public PRNG {
private:
    vector<unsigned long long> buffer;
    int index1, index2;
    static const int J = 55;
    static const int K = 24;

public:
    AdditiveLaggedFibonacci(unsigned long long s) : PRNG(s) {
        buffer.resize(J);
        unsigned long long temp = s;

        for (int i = 0; i < J; i++) {
            temp = (1103515245 * temp + 12345) % 2147483648;
            buffer[i] = temp;
        }

        index1 = 0;
        index2 = J - K;
    }

    int generate(int min_val, int max_val) override {
        unsigned long long result = (buffer[index1] + buffer[index2]) % 2147483648;

        buffer[index1] = result;

        index1 = (index1 + 1) % J;
        index2 = (index2 + 1) % J;

        return min_val + static_cast<int>(result % (max_val - min_val + 1));
    }

    string get_name() const override {
        return "Аддитивный генератор (Lagged Fibonacci)";
    }
};

// ============================================================
// ФУНКЦИЯ АНАЛИЗА РАВНОМЕРНОСТИ
// ============================================================
void analyze_distribution(PRNG& gen, int count, int min_val, int max_val, int buckets) {
    vector<int> freq(buckets, 0);
    int range = max_val - min_val + 1;
    double expected = static_cast<double>(count) / buckets;

    for (int i = 0; i < count; ++i) {
        int num = gen.generate(min_val, max_val);
        int idx = (num - min_val) * buckets / range;
        if (idx >= buckets) idx = buckets - 1;
        freq[idx]++;
    }

    cout << "\n";
    cout << "==================================================\n";
    cout << "Анализ: " << gen.get_name() << "\n";
    cout << "==================================================\n";
    cout << "Чисел сгенерировано: " << count << "\n";
    cout << "Интервалов: " << buckets << "\n";
    cout << "Ожидаемое в каждом: " << expected << "\n\n";

    double total_deviation = 0.0;

    cout << "Интервал | Получено | Отклонение\n";
    cout << "---------|----------|------------\n";

    for (int i = 0; i < buckets; ++i) {
        double dev = fabs(freq[i] - expected);
        double dev_percent = (dev / expected) * 100.0;
        total_deviation += dev_percent;

        cout << "   " << (i + 1) << "       |    " << freq[i] << "     | " << dev_percent << "%\n";
    }

    cout << "---------|----------|------------\n";
    double avg_dev = total_deviation / buckets;
    cout << "Среднее отклонение: " << avg_dev << "%\n";

    cout << "Качество: ";
    if (avg_dev < 3.0)
        cout << "Отличное\n";
    else if (avg_dev < 7.0)
        cout << "Хорошее\n";
    else if (avg_dev < 15.0)
        cout << "Удовлетворительное\n";
    else
        cout << "Плохое\n";
}

// ============================================================
// ГЛАВНАЯ ФУНКЦИЯ
// ============================================================
int main() {
    setlocale(LC_ALL, "");

    cout << "\n";
    cout << "==================================================\n";
    cout << "   ГЕНЕРАТОР ПСЕВДОСЛУЧАЙНЫХ ЧИСЕЛ\n";
    cout << "==================================================\n\n";

    int choice = 0;
    unsigned long long seed;
    int count, min_val, max_val;

    cout << "Введите начальное значение (seed): ";
    cin >> seed;

    cout << "Количество чисел для генерации: ";
    cin >> count;

    cout << "Диапазон генерации [min max]: ";
    cin >> min_val >> max_val;

    if (min_val > max_val) {
        cout << "Ошибка: min не может быть больше max!\n";
        return 1;
    }

    if (count <= 0) {
        cout << "Ошибка: количество должно быть положительным!\n";
        return 1;
    }

    do {
        cout << "\n";
        cout << "--------------------------------------------------\n";
        cout << "ВЫБЕРИТЕ МЕТОД ГЕНЕРАЦИИ:\n";
        cout << "--------------------------------------------------\n";
        cout << "1. Метод середины квадрата\n";
        cout << "2. Линейный конгруэнтный метод (LCG)\n";
        cout << "3. Мультипликативный метод\n";
        cout << "4. XORshift\n";
        cout << "5. PCG\n";
        cout << "6. Аддитивный генератор\n";
        cout << "0. Выход\n";
        cout << "--------------------------------------------------\n";
        cout << "Ваш выбор: ";
        cin >> choice;

        PRNG* generator = nullptr;

        switch (choice) {
        case 1:
            generator = new MiddleSquare(seed);
            break;
        case 2:
            generator = new LCG(seed);
            break;
        case 3:
            generator = new Multiplicative(seed);
            break;
        case 4:
            generator = new XORshift(seed);
            break;
        case 5:
            generator = new PCG(seed);
            break;
        case 6:
            generator = new AdditiveLaggedFibonacci(seed);
            break;
        case 0:
            cout << "\nЗавершение работы...\n";
            return 0;
        default:
            cout << "\nНеверный ввод!\n";
            continue;
        }

        analyze_distribution(*generator, count, min_val, max_val, 10);

        delete generator;

        cout << "\nНажмите Enter для продолжения...";
        cin.ignore();
        cin.get();

    } while (choice != 0);

    return 0;
}