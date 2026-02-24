#ifndef _TRACKER_CONTROLLER_HPP_
#define _TRACKER_CONTROLLER_HPP_

#include "Common/Quaternion.hpp"
#include <algorithm>
#include <cmath>
#include <devguid.h>
#include <iostream>
#include <setupapi.h>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>

#pragma comment(lib, "setupapi.lib")

struct GyroData {
	int time;
	float ax, ay, az;
	float gx, gy, gz;
};

struct COMInfo {
	std::string port; // Ej: COM3
	std::string name; // Ej: USB-SERIAL CH340 (COM3)
};

#define SIZE_1 390

class TrackerController {
public:
	TrackerController() = default;

	~TrackerController() {
		disconnect();
	}

	TrackerController(const TrackerController &) = delete;
	TrackerController & operator=(const TrackerController &) = delete;

	bool setup(const std::wstring & portName, DWORD baudRate = 115200) {
		disconnect();
		hSerial = CreateFileW(
			portName.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (hSerial == INVALID_HANDLE_VALUE) {
			PrintLastError("CreateFile");
			return false;
		}

		std::cout << "Serial port opened\n";

		DCB dcbSerialParams = { 0 };
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

		if (!GetCommState(hSerial, &dcbSerialParams)) {
			PrintLastError("GetCommState");
			CloseHandle(hSerial);
			hSerial = INVALID_HANDLE_VALUE;
			return false;
		}

		dcbSerialParams.BaudRate = baudRate;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.StopBits = ONESTOPBIT;
		dcbSerialParams.Parity = NOPARITY;
		dcbSerialParams.fBinary = TRUE;
		dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;
		dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE;

		if (!SetCommState(hSerial, &dcbSerialParams)) {
			PrintLastError("SetCommState");
			CloseHandle(hSerial);
			hSerial = INVALID_HANDLE_VALUE;
			return false;
		}

		COMMTIMEOUTS timeouts = { 0 };
		timeouts.ReadIntervalTimeout = 20;
		timeouts.ReadTotalTimeoutMultiplier = 1;
		timeouts.ReadTotalTimeoutConstant = 50;
		SetCommTimeouts(hSerial, &timeouts);

		PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

		std::cout << "Serial configured\n";
		initialized = true;

		return true;
	}

	void disconnect() {
		if (hSerial != INVALID_HANDLE_VALUE) {
			CloseHandle(hSerial);
			hSerial = INVALID_HANDLE_VALUE;
			initialized = false;
			std::cout << "Serial port closed\n";
		}
	}

	bool leerLinea() {
		if (!initialized || hSerial == INVALID_HANDLE_VALUE) return false;

		char receive_data_buffer[300] = { 0 };
		DWORD bytesRead = 0;

		bool success = ReadFile(
			hSerial,
			receive_data_buffer,
			sizeof(receive_data_buffer),
			&bytesRead,
			NULL);

		if (!success) {
			std::cout << "ERROR EN LECTURA\n";
			return false;
		}

		if (bytesRead == 0) return false;

		size_t j = 0;
		int z = 0;
		int num_comas = 0;
		double array[10] = { 0 };
		char auxiliar[300] = { 0 };

		while (j < bytesRead) {
			char c = receive_data_buffer[j];

			if (c != '\r' && c != '\n') {
				auxiliar[z] = c;
				if (c == ',') num_comas++;
				z++;
			} else {
				if (num_comas == 4) {
					auxiliar[z] = '\0';
					char * token = strtok(auxiliar, ",");
					int pos = 0;
					bool linea_correcta = true;

					while (token != NULL && pos < 10) {
						char * pEnd;
						array[pos] = strtod(token, &pEnd);

						if (pos == 0 && array[pos] <= 1000)
							linea_correcta = false;

						token = strtok(NULL, ",");
						pos++;
					}

					if (linea_correcta) {
						float q1 = static_cast<float>(array[1]);
						float q2 = static_cast<float>(array[2]);
						float q3 = static_cast<float>(array[3]);
						float q0 = sqrt(1.0f - (q1 * q1 + q2 * q2 + q3 * q3));
						ultimoQuat = { q0, q1, q2, q3 };
						return true;
					}
				}
				memset(auxiliar, 0, sizeof(auxiliar));
				z = 0;
				num_comas = 0;
			}
			j++;
		}

		return false;
	}

	void calcularEuler(double & yaw, double & pitch, double & roll) {
		double qw = ultimoQuat.w;
		double qx = ultimoQuat.x;
		double qy = ultimoQuat.y;
		double qz = ultimoQuat.z;

		double t0 = +2.0 * (qw * qx + qy * qz);
		double t1 = +1.0 - 2.0 * (qx * qx + qy * qy);
		roll = atan2(t0, t1) * 180.0 / M_PI;

		double t2 = +2.0 * (qw * qy - qx * qz);
		t2 = std::clamp(t2, -1.0, 1.0);
		pitch = asin(t2) * 180.0 / M_PI;

		double t3 = +2.0 * (qw * qz + qx * qy);
		double t4 = +1.0 - 2.0 * (qy * qy + qz * qz);
		yaw = atan2(t3, t4) * 180.0 / M_PI;
	}

	CQuaternion getUltimoQuat() const { return ultimoQuat; }

	// -------------------------------------------------
	// Listar COMs con nombre descriptivo
	// -------------------------------------------------
	static std::vector<COMInfo> listarCOMsConNombre() {
		std::vector<COMInfo> coms;

		HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT);
		if (hDevInfo == INVALID_HANDLE_VALUE) return coms;

		SP_DEVINFO_DATA devInfoData;
		devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); i++) {
			char friendlyName[256] = { 0 };
			if (SetupDiGetDeviceRegistryPropertyA(
					hDevInfo,
					&devInfoData,
					SPDRP_FRIENDLYNAME,
					NULL,
					(PBYTE)friendlyName,
					sizeof(friendlyName),
					NULL)) {
				std::string fname(friendlyName);
				auto start = fname.find("COM");
				if (start != std::string::npos) {
					auto end = fname.find(")", start);
					std::string port = fname.substr(start, end - start);
					coms.push_back({ port, fname });
				}
			}
		}

		SetupDiDestroyDeviceInfoList(hDevInfo);
		return coms;
	}

void calibrarSensoresQuat(int muestrasQuieto = 50, int muestrasMov = 100) {
		std::cout << "=== Calibración de orientación (cuaterniones) ===\n";

		std::cout << "Mantenga el sensor quieto...\n";

		CQuaternion sumaQuat;
		int leidas = 0;
		while (leidas < muestrasQuieto) {
			if (leerLinea()) {
				sumaQuat.x += ultimoQuat.x;
				sumaQuat.y += ultimoQuat.y;
				sumaQuat.z += ultimoQuat.z;
				sumaQuat.w += ultimoQuat.w;
				leidas++;
			}
		}

		CQuaternion qQuieto;
		qQuieto.w = sumaQuat.w / muestrasQuieto;
		qQuieto.x = sumaQuat.x / muestrasQuieto;
		qQuieto.y = sumaQuat.y / muestrasQuieto;
		qQuieto.z = sumaQuat.z / muestrasQuieto;

		std::cout << "Valor promedio en reposo (referencia): "
				  << "q0=" << qQuieto.w << ", "
				  << "q1=" << qQuieto.x << ", "
				  << "q2=" << qQuieto.y << ", "
				  << "q3=" << qQuieto.z << "\n";

		std::cout << "Gire el sensor lentamente en todas las direcciones (tipo infinito)...\n";

		std::vector<CQuaternion> datasetMov;
		leidas = 0;
		while (leidas < muestrasMov) {
			if (leerLinea()) {
				datasetMov.push_back(ultimoQuat);
				leidas++;
			}
		}

		std::cout << "Calibración completada. Se recolectaron "
				  << datasetMov.size() << " cuaterniones en movimiento.\n";

	}



private:
	HANDLE hSerial = INVALID_HANDLE_VALUE;
	bool initialized = false;
	CQuaternion ultimoQuat;

	void PrintLastError(const char * functionName) {
		DWORD error = GetLastError();
		char message[256];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error, 0, message, sizeof(message), NULL);
		std::cout << functionName << " failed. Error " << error << ": " << message << std::endl;
	}
};

#endif
