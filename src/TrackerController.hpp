#ifndef _TRACKER_CONTROLLER_HPP_
#define _TRACKER_CONTROLLER_HPP_

#include "Common/Quaternion.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>

struct GyroData {
	int time;
	float ax, ay, az;
	float gx, gy, gz;
};

class TrackerController {
public:
	// Singleton
	static TrackerController & getInstance(const std::wstring & portName , DWORD baudRate = 115200) {
		static TrackerController instance(portName, baudRate);
		return instance;
	}

	// No copiar ni asignar
	TrackerController(const TrackerController &) = delete;
	TrackerController & operator=(const TrackerController &) = delete;

	~TrackerController() {
		if (hSerial != INVALID_HANDLE_VALUE) {
			CloseHandle(hSerial);
			std::cout << "Serial port closed\n";
		}
	}

	// Leer línea completa y mostrarla RAW
	bool leerLinea() {
		if (hSerial == INVALID_HANDLE_VALUE) return false;

		char receive_data_buffer[256] = { 0 };
		DWORD bytesRead = 0;

		bool success = ReadFile(
			hSerial,
			receive_data_buffer,
			sizeof(receive_data_buffer) - 1,
			&bytesRead,
			NULL);

		if (!success || bytesRead == 0) return false;

		receive_data_buffer[bytesRead] = '\0'; 

		int j = 0;
		int z = 0;
		int num_comas = 0;
		char auxiliar[300] = { 0 };
		double array[10] = { 0 };

		while (j < bytesRead) {
			char c = receive_data_buffer[j];

			if (c != '\r') {
				if (c != '\n') {
					auxiliar[z++] = c;
					if (c == ',') num_comas++;
				}
			} else {
				if (num_comas == 4) { // línea válida
					auxiliar[z] = '\0';

					// Parsear números
					char * token = strtok(auxiliar, ",");
					int pos = 0;
					bool correcto = true;
					while (token != NULL && pos < 10) {
						char * pEnd;
						array[pos] = strtod(token, &pEnd);

						if (pos == 0 && array[pos] <= 1000) { // primer valor, timestamp/id
							correcto = false;
						}

						token = strtok(NULL, ",");
						pos++;
					}

					if (correcto) {
						// Guardar los valores Q1,Q2,Q3 en tu quaternion
						float q1 = static_cast<float>(array[1]);
						float q2 = static_cast<float>(array[2]);
						float q3 = static_cast<float>(array[3]);
						float q0 = sqrt(1.0f - (q1 * q1 + q2 * q2 + q3 * q3));

						ultimoQuat = { q0, q1, q2, q3 };

						// Mostrar debug
						/* std::cout << "Leído: " << auxiliar << "\n";
						double roll, pitch, yaw;
						calcularEuler(yaw, pitch, roll);
						std::cout << "Yaw: " << yaw
								  << " Pitch: " << pitch
								  << " Roll: " << roll << "\n";*/
					}
				}
				// Reset para la siguiente línea
				memset(auxiliar, 0, sizeof(auxiliar));
				z = 0;
				num_comas = 0;
			}

			j++;
		}

		return true;
	}

	// Convertir a Euler (roll/pitch/yaw) en grados
	void calcularEuler(double & yaw, double & pitch, double & roll) {
		double qw = ultimoQuat.w;
		double qx = ultimoQuat.x;
		double qy = ultimoQuat.y;
		double qz = ultimoQuat.z;

		// Roll (x-axis)
		double t0 = +2.0 * (qw * qx + qy * qz);
		double t1 = +1.0 - 2.0 * (qx * qx + qy * qy);
		roll = atan2(t0, t1) * 180.0 / M_PI;

		// Pitch (y-axis)
		double t2 = +2.0 * (qw * qy - qx * qz);
		t2 = std::clamp(t2, -1.0, 1.0);
		pitch = asin(t2) * 180.0 / M_PI;

		// Yaw (z-axis)
		double t3 = +2.0 * (qw * qz + qx * qy);
		double t4 = +1.0 - 2.0 * (qy * qy + qz * qz);
		yaw = atan2(t3, t4) * 180.0 / M_PI;
	}

	CQuaternion getUltimoQuat() const {
		return ultimoQuat;
	}

	GyroData ultimoGyro;

private:
	HANDLE hSerial = INVALID_HANDLE_VALUE;
	std::string linea;
	CQuaternion ultimoQuat;

	TrackerController(const std::wstring & portName, DWORD baudRate) {
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
			return;
		}

		std::cout << "Serial port opened\n";

		DCB dcbSerialParams = { 0 };
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

		if (!GetCommState(hSerial, &dcbSerialParams)) {
			PrintLastError("GetCommState");
			CloseHandle(hSerial);
			hSerial = INVALID_HANDLE_VALUE;
			return;
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
			return;
		}

		COMMTIMEOUTS timeouts = { 0 };
		timeouts.ReadIntervalTimeout = 20;
		timeouts.ReadTotalTimeoutMultiplier = 1;
		timeouts.ReadTotalTimeoutConstant = 50;
		SetCommTimeouts(hSerial, &timeouts);

		PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

		std::cout << "Serial configured\n";
	}

	void PrintLastError(const char * functionName) {
		DWORD error = GetLastError();
		char message[256];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error, 0, message, sizeof(message), NULL);
		std::cout << functionName << " failed. Error " << error << ": " << message << std::endl;
	}
};

#endif
