
#define MODELARX_H

#include "ObiektSISO.h"
#include <vector>
#include <deque>
#include <random>
#include <string>
#include "Wielomian.h"

class ModelARX : public ObiektSISO
{
private:
    Wielomian m_A;   //  Współczynniki wielomianu A
    Wielomian m_B;   //  Współczynniki wielomianu B
    int m_k;         //  Opóźnienie transportowe
    double m_stdDev; //  Odchylenie standardowe dla generowania zakłóceń

    std::deque<double> m_outputMemory; //  Bufor pamięci wyjść
    std::deque<double> m_inputMemory;  // Bufor pamięci wejść
    std::deque<double> m_delayBuffer;  // Bufor opóźnionych wejść

    std::default_random_engine m_generator;          //  Generator losowy do zakłóceń
    std::normal_distribution<double> m_distribution; //  Rozkład normalny dla zakłóceń

    double generujZaklocenie();

    //  Funkcja pomocnicza do inicjalizacji modelu z współczynników A, B, k, stdDev
    void inicjalizuj(const std::vector<double> &a, const std::vector<double> &b, int k, double stdDev);

public:
    //  Konstruktor parametryczny
    ModelARX(const std::vector<double> &a, const std::vector<double> &b, int k = 1, double stdDev = 0.0);

    //  Konstruktor deserializujący
    explicit ModelARX(const std::string &data);

    //  Usuwam konstruktor domyślny
    ModelARX() = delete;

    // Destruktor
    virtual ~ModelARX() override = default;

    void setA(const std::vector<double> &a);
    void setB(const std::vector<double> &b);
    void setK(int k);
    void setStdDev(double stdDev);

    //  Metoda symulacji modelu ARX
    virtual double symuluj(double u) override;

    //  Serializacja modelu ARX do tekstu
    std::string serializuj() const;
};
