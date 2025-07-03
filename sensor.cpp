#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <openssl/sha.h> //para usar SHA256

// Estructura de los datos
#pragma pack(push, 1) //evita padding
struct SensorData {
    int16_t id;
    int64_t timestamp;
    float temperatura;
    float presion;
    float humedad;
};
#pragma pack(pop)

// Función para crear un hash SHA256
void crearHash(const SensorData& data, unsigned char hash[SHA256_DIGEST_LENGTH]) {
    SHA256((const unsigned char*)&data, sizeof(SensorData), hash);
}

void printHash(unsigned char hash[SHA256_DIGEST_LENGTH]) {
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        printf("%02x", hash[i]);
    printf("\n");
}

int main() {
    // Inicializar Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    // Crear datos simulados
    SensorData data;
    data.id = 1;
    data.timestamp = std::time(nullptr);
    data.temperatura = 22.5f + (rand() % 100) / 10.0f;
    data.presion = 1013.25f + (rand() % 50) / 10.0f;
    data.humedad = 40.0f + (rand() % 100) / 10.0f;

    // Crear firma
    unsigned char hash[SHA256_DIGEST_LENGTH];
    crearHash(data, hash);

    // Conectarse al servidor intermedio
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); // Puerto del servidor intermedio
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); // IP local

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "Connect failed: " << WSAGetLastError() << "\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Enviar datos + firma
    send(sock, (const char*)&data, sizeof(data), 0);
    send(sock, (const char*)hash, SHA256_DIGEST_LENGTH, 0);

    std::cout << "Datos enviados correctamente.\n";
    std::cout << "Hash: ";
    printHash(hash);

    closesocket(sock);
    WSACleanup();
    return 0;
}
