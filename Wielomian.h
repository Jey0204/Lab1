#pragma once

#include <vector>
#include <deque>
#include <numeric>

class Wielomian
{
private:
    std::vector<double> wspolczynniki;

public:
    // Konstruktor domyślny
    Wielomian() = default;

    // Konstruktor inicjalizujący
    explicit Wielomian(const std::vector<double> &wsp)
        : wspolczynniki(wsp) {} // Inicjalizuje współczynniki wielomianu

    // Umożliwia zaktualizowanie współczynników wielomianu na nowe wartości.
    void ustawWspolczynniki(const std::vector<double> &wsp)
    {
        wspolczynniki = wsp;
    }

    // Zwraca wektor współczynników, aby móc je odczytać
    const std::vector<double> &pobierzWspolczynniki() const
    {
        return wspolczynniki;
    }

    // Na podstawie danych wejściowych (np. próbek wejściowych w czasie) oblicza wynik wielomianu.
    double oblicz(const std::deque<double> &dane) const
    {
        if (wspolczynniki.size() > dane.size())
            return 0.0;

        return std::inner_product(wspolczynniki.begin(), wspolczynniki.end(), dane.begin(), 0.0);
    }

    // Stopień wielomianu
    size_t stopien() const
    {
        return wspolczynniki.size(); // Zwraca liczbę współczynników
    }
};
