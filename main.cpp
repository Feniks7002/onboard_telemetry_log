#pragma once
#include <cstdint>

using namespace std;

enum RocketState : uint8_t {
    DISARM = 0 ,
    ARM = 1 ,
    FLIGHT = 2 ,
    PARACHUTE = 3 ,
    LANDED = 4
};

class TelemetryBlackBox {
private: 
    // Inicjalizacja zmiennych

    // Rozmiar bufora kołowego
    static const uint8_t bufferSize = 16;

    RocketState state;
    uint8_t currentIndex;
    uint8_t framesCount;
    uint16_t altitude;
    uint32_t frames[bufferSize] = {};

    bool parachute;
    bool baro_error;
    
public :
    // Konstruktor inicjalizujący wartości klasy
    TelemetryBlackBox() {
        state = DISARM;
        currentIndex = 0;
        framesCount = 0;
        altitude = 0;
        parachute = false;
        baro_error = false;
    }

    // Aktualizuje aktualny stan rakiety
    void onRocketStateChange ( RocketState state ) {
        // Użycie "this->" umożliwia działanie na tych samych nazwach zmiennych w jednej funkcji
        this->state = state;
    }

    // Ustawia flagę informującą o wdrożeniu spadochronu
    void onParachuteDeployed () {
        parachute = true;
    }

    /**
    * @brief Aktualizuje dane z barometru.
    * @param error - Informuje czy wystąpił błąd barometru
    * @param value - Odczyt wysokości w stopach
    */
    void onBaroReading ( bool error , uint16_t value ) {
        baro_error = error;
        altitude = value;
    }

    /**
    * @brief Tworzy 32-bitową ramkę telemetryczną,
    * oblicza sumę kontrolną i zapisuje ją do bufora.
    */
    void recordCurrentState () {
        uint32_t frame = 0;
        uint16_t encodedAltitude = altitude;

        // Zapis stanu rakiety w bitach 0–2
        // Użyte zostało rzutowanienie typu zmiennej na uint32_t z uint8_t, aby zapewnić brak błędów przy przesunięciu bitowym
        frame |= (uint32_t)state;

        // Ustawienie flagi spadochronu
        if (parachute) {
            frame |= (1u << 3);
        }

        // Ustawienie flagi błędu barometru
        if (baro_error) {
            frame |= (1u << 4);
            encodedAltitude = 0xFFFF;
        } else if (state == DISARM || state == LANDED) {
            encodedAltitude = 0;
        }

        // Zapis wysokości w bitach 8–23
        // Użyte zostało rzutowanienie typu zmiennej na uint32_t z uint16_t, aby zapewnić brak błędów przy przesunięciu bitowym
        frame |= ((uint32_t)encodedAltitude << 8);

        // Wyodrębnienie pierwszych trzech bajtów ramki
        uint8_t bajt0 = frame & 0xFF;
        uint8_t bajt1 = (frame >> 8) & 0xFF;
        uint8_t bajt2 = (frame >> 16) & 0xFF;
        
        // Obliczenie sumy kontrolnej XOR
        uint8_t checksum = bajt0 ^ bajt1 ^ bajt2;

        // Zapis sumy kontrolnej w bitach 24–31
        // Użyte zostało rzutowanienie typu zmiennej na uint32_t z uint8_t, aby zapewnić brak błędów przy przesunięciu bitowym
        frame |= ((uint32_t)checksum << 24);
        
        // Zapis ramki do aktualnej pozycji bufora
        frames[currentIndex] = frame;

        // Przesunięcie indeksu na kolejną pozycję
        currentIndex = (currentIndex + 1) % bufferSize;

        // Licznik nie może przekroczyć rozmiaru bufora
        if (currentIndex < bufferSize) {
            framesCount++;
        }
    }

    /**
    * @brief Funkcja pobiera ostatnią ramkę z bufora kołowego, dekoduje ją oraz loguje zdekodowane wartości w czytelny sposób na konsolę.
    * @return Ostatnia ramka zapisana do bufora, jeśli xor się zgadza .
    0 xFFFFFFFF jeśli suma kontrolna jest niepoprawna .
    */
    uint32_t getLastDataframe () {
        if (framesCount == 0) {
            return 0xFFFFFFFF;
        }

        // Wyznaczenie indeksu ostatnio zapisanej ramki
        uint8_t lastIndex = (currentIndex - 1 + bufferSize) % bufferSize;
        uint32_t lastFrame = frames[lastIndex];

        // Wyodrębnienie pierwszych trzech bajtów
        uint8_t bajt0 = lastFrame & 0xFF;
        uint8_t bajt1 = (lastFrame >> 8) & 0xFF;
        uint8_t bajt2 = (lastFrame >> 16) & 0xFF;

        // Odczyt oraz obliczenie wartości sumy kontrolnej xor 
        uint8_t checksum = (lastFrame >> 24) & 0xFF;
        uint8_t checksum2 = bajt0 ^ bajt1 ^ bajt2;

        if (checksum == checksum2) {
            return lastFrame;
        } else {
            return 0xFFFFFFFF;
        }
    }
};
