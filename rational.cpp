#include <iostream>

class Rational {
  public:  
    int num, den;

    Rational() : num(0), den(1) {}

    Rational(int value) : num(value), den(1) {}

    Rational(int num, int den) : num(num), den(den) {
        if (this->den < 0) {
            this->den = -this->den;
            this->num = -this->num;
        }
        reduce();
    }
    

    double toDouble() {
        return num / (double)den;
    }

    int get_num() { return num; }

    int get_den() { return den; }

    Rational operator=(const Rational &other) {
        num = other.num;
        den = other.den;
        return *this;
    }

    Rational operator=(const int value) {
        num = value;
        den = 1;
        return *this;
    }

    Rational operator+(const Rational &other) const {
        return Rational{num * other.den + other.num * den, den * other.den};
    }

    Rational operator+=(const Rational &other) {
        *this = *this + other;
        return *this;
    }

    Rational operator-(const Rational &other) const {
        return -other + *this;
    }

    Rational operator-=(const Rational &other) {
        *this = *this - other;
        return *this;
    }

    Rational operator*(const Rational &other) const {
        return Rational{num * other.num, den * other.den};
    }

    Rational operator*=(const Rational &other) {
        *this = *this * other;
        return *this;
    }

    Rational operator/(const Rational &other) const {
        return Rational{num * other.den, den * other.num};
    }

    Rational operator/=(const Rational &other) {
        *this = *this / other;
        return *this;
    }

    Rational operator-() const {
        return Rational{-num, den};
    }

    bool operator<(const Rational &other) const {
        return num * other.den < other.num * den;
    }

    bool operator>(const Rational &other) const {
        return other < *this;
    }

    bool operator<=(const Rational &other) {
        return !(other < *this);
    }

    bool operator>=(const Rational &other) {
        return !(*this < other);
    }

    bool operator==(const Rational &other) {
        return num == other.num && den == other.den;
    }

    bool operator!=(const Rational &other) {
        return !(*this == other);
    }

private:

    void reduce() {
        auto g = gcd(num < 0 ? -num : num, den);
        num /= g;
        den /= g;
    }

    int gcd(int a, int b) {
        while (a) {
            auto t = b % a;
            b = a;
            a = t;
        }
        return b;
    }

};
