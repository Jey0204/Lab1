
#include "ModelARX.h"
#include <iostream>
#include <stdexcept>
#include <numeric>
#include <functional>
#include <cmath>
#include <sstream>
#include <limits>
#include <chrono>

void ModelARX::inicjalizuj(const std::vector<double> &a, const std::vector<double> &b, int k_val, double stdDev_val)
{
    setA(a);               //  Ustawienie współczynników mianownika A
    setB(b);               //  Ustawienie współczynników licznika B
    setK(k_val);           //  Ustawienie opóźnienia transportowego
    setStdDev(stdDev_val); //  Ustawienie mocy zakłócenia

    //  Losowe ziarno do generatora zakłóceń
    m_generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
}

//  Generowanie zakłócenia - domyślnie 0, jeśli stdDev == 0
double ModelARX::generujZaklocenie()
{
    if (m_stdDev > std::numeric_limits<double>::epsilon())
    {
        return m_distribution(m_generator); //  Losowe zakłócenie
    }
    else
    {
        return 0.0;
    }
}

//  Konstruktor parametryczny
ModelARX::ModelARX(const std::vector<double> &a, const std::vector<double> &b, int k, double stdDev)
{
    inicjalizuj(a, b, k, stdDev);
}

//  Konstruktor deserializujący
ModelARX::ModelARX(const std::string &data)
{
    std::istringstream iss(data);
    std::string line, key;
    std::vector<double> tempA, tempB;
    int tempK = 1;
    double tempStdDev = 0.0;
    bool foundA = false, foundB = false, foundK = false, foundStdDev = false;

    while (std::getline(iss, line))
    {
        std::istringstream lineStream(line);
        if (line.empty() || line[0] == '#')
            continue; // Pomijanie komentarzy

        lineStream >> key;

        if (key == "A:")
        {
            tempA.clear();
            double val;
            while (lineStream >> val)
                tempA.push_back(val);
            foundA = true;
        }
        else if (key == "B:")
        {
            tempB.clear();
            double val;
            while (lineStream >> val)
                tempB.push_back(val);
            foundB = true;
        }
        else if (key == "k:")
        {
            if (!(lineStream >> tempK))
                throw std::runtime_error("Błąd parsowania k");
            foundK = true;
        }
        else if (key == "stdDev:")
        {
            if (!(lineStream >> tempStdDev))
                throw std::runtime_error("Błąd parsowania stdDev");
            foundStdDev = true;
        }
        else if (key == "Type:")
        {
            std::string typeVal;
            lineStream >> typeVal;
            if (typeVal != "ModelARX")
                throw std::runtime_error("Typ niezgodny: oczekiwano ModelARX");
        }
    }

    if (!foundA || !foundB || !foundK || !foundStdDev)
    {
        std::string missing;
        if (!foundA)
            missing += " A";
        if (!foundB)
            missing += " B";
        if (!foundK)
            missing += " k";
        if (!foundStdDev)
            missing += " stdDev";
        throw std::runtime_error("Brakuje danych w deserializacji:" + missing);
    }

    inicjalizuj(tempA, tempB, tempK, tempStdDev);
}

// Ustawienie współczynników wielomianu A
void ModelARX::setA(const std::vector<double> &a)
{
    m_A.ustawWspolczynniki(a);
    m_outputMemory.resize(a.size(), 0.0);
}

//  Ustawienie współczynników wielomianu B
void ModelARX::setB(const std::vector<double> &b)
{
    m_B.ustawWspolczynniki(b);
    m_inputMemory.resize(b.size(), 0.0);
}

// Ustawienie opóźnienia transportowego
void ModelARX::setK(int k)
{
    m_k = std::max(1, k);
    m_delayBuffer.assign(m_k, 0.0);
}

//  Ustawienie odchylenia standardowego zakłócenia
void ModelARX::setStdDev(double stdDev)
{
    m_stdDev = std::max(0.0, stdDev);
    m_distribution = std::normal_distribution<double>(0.0, m_stdDev);
}

double ModelARX::symuluj(double u)
{
    //   wejście do bufora opóźnień
    m_delayBuffer.push_back(u);
    double u_delayed = 0.0;

    // Pobieramy opóźnione wejście
    if (m_delayBuffer.size() > m_k)
    {
        u_delayed = m_delayBuffer.front();
        m_delayBuffer.pop_front();
    }

    //  Aktualizacja wejśća
    if (!m_inputMemory.empty())
    {
        m_inputMemory.push_front(u_delayed);
        m_inputMemory.pop_back();
    }

    //  Obliczenie sumy wpływu wejść i wyjść wg równania różnicowego modelu ARX
    double x_sum = m_B.oblicz(m_inputMemory);    // B(q) * u(k)
    double ar_sum = -m_A.oblicz(m_outputMemory); // -A(q) * y(k)
    double noise = generujZaklocenie();

    double y = x_sum + ar_sum + noise;

    //  Aktualizacja wyjść
    if (!m_outputMemory.empty())
    {
        m_outputMemory.push_front(y);
        m_outputMemory.pop_back();
    }

    return y;
}

//  Serializacja obiektu do napisu tekstowego
std::string ModelARX::serializuj() const
{
    std::ostringstream oss;
    oss.precision(std::numeric_limits<double>::max_digits10);

    oss << "Type: ModelARX\n";

    oss << "A:";
    for (double coeff : m_A.pobierzWspolczynniki())
        oss << " " << coeff;
    oss << "\n";

    oss << "B:";
    for (double coeff : m_B.pobierzWspolczynniki())
        oss << " " << coeff;
    oss << "\n";

    oss << "k: " << m_k << "\n";
    oss << "stdDev: " << m_stdDev << "\n";

    return oss.str();
}
