#pragma once
#include <QString>
#include <vector>

// ============================================================
// БАЗОВЫЙ КЛАСС
// ============================================================
class PRNG {
protected:
    unsigned long long seed;
public:
    PRNG(unsigned long long s) : seed(s) {}
    virtual ~PRNG() {}

    virtual int generate(int min_val, int max_val) = 0;
    virtual QString name() const = 0;
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

    QString name() const override { return "Середина квадрата"; }
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

    QString name() const override { return "LCG (линейный конгруэнтный)"; }
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

    QString name() const override { return "Мультипликативный"; }
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

    QString name() const override { return "XORshift"; }
};

// ============================================================
// 5. PCG
// ============================================================
class PCG : public PRNG {
private:
    unsigned long long state;
    static const unsigned long long mult = 6364136223846793005ULL;
    static const unsigned long long inc  = 1442695040888963407ULL;

public:
    PCG(unsigned long long s) : PRNG(s), state(s) {
        state = (state * mult + inc) & 0xFFFFFFFFFFFFFFFFULL;
        state = (state * mult + inc) & 0xFFFFFFFFFFFFFFFFULL;
    }

    int generate(int min_val, int max_val) override {
        unsigned long long old = state;
        state = (old * mult + inc) & 0xFFFFFFFFFFFFFFFFULL;
        unsigned int xs = static_cast<unsigned int>(((old >> 18) ^ old) >> 27);
        unsigned int rot = static_cast<unsigned int>(old >> 59);
        unsigned int result = (xs >> rot) | (xs << ((32 - rot) & 31));
        return min_val + static_cast<int>(result % (max_val - min_val + 1));
    }

    QString name() const override { return "PCG (современный)"; }
};

// ============================================================
// 6. АДДИТИВНЫЙ ГЕНЕРАТОР (Lagged Fibonacci)
// ============================================================
class AdditiveLF : public PRNG {
private:
    std::vector<unsigned long long> buf;
    int i1, i2;
    static const int J = 55, K = 24;

public:
    AdditiveLF(unsigned long long s) : PRNG(s), buf(J), i1(0), i2(J - K) {
        unsigned long long tmp = s;
        for (int i = 0; i < J; i++) {
            tmp = (1103515245ULL * tmp + 12345) % 2147483648ULL;
            buf[i] = tmp;
        }
    }

    int generate(int min_val, int max_val) override {
        unsigned long long r = (buf[i1] + buf[i2]) % 2147483648ULL;
        buf[i1] = r;
        i1 = (i1 + 1) % J;
        i2 = (i2 + 1) % J;
        return min_val + static_cast<int>(r % (max_val - min_val + 1));
    }

    QString name() const override { return "Аддитивный (Lagged Fibonacci)"; }
};
