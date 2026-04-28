/**
 * \class TrackerController
 *
 * \brief Manages serial communication and orientation tracking from an IMU-based motion tracker.
 *
 * \details This class provides low-level control and processing of data received from a
 * serial-connected inertial measurement unit (IMU). 
 *
 * \note The implementation uses Windows Serial API (CreateFile, ReadFile, SetupAPI)
 * for communication and is therefore platform-dependent (Windows only).
 *
 * \note Calibration routines include:
 *  - Full sensor calibration using figure-8 motion patterns.
 *  - Center offset calibration based on pitch ("yes") and yaw ("no") head motions.
 *
 * \date April 2026
 *
 * \author R. Liceras-Ramírez  
 * \b Tutor: A. Reyes-Lecuona and D. Gonzalez-Toledo  
 * \b Institution: Universidad de Málaga
 *
 * \b Context:
 * This class was developed as part of the Bachelor's Thesis (TFG)
 * It is used within a real-time tracking and orientation system based on IMU
 * sensors for motion capture and spatial audio applications.
 * 
 * \b Licence:
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 */

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
	std::string port; 
	std::string name; 
};

class TrackerController {
public:

	TrackerController(): initialized(false), hSerial(INVALID_HANDLE_VALUE){ }

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

		// Prevents ReadFile from blocking indefinitely while waiting for data
		COMMTIMEOUTS timeouts = { 0 };
		timeouts.ReadIntervalTimeout = 20;
		timeouts.ReadTotalTimeoutConstant = 50;
		SetCommTimeouts(hSerial, &timeouts);

		//Clean buffers
		PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

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

	bool isConnected() const {
		return initialized && hSerial != INVALID_HANDLE_VALUE;
	}

	// Line format: time, q0, q1, q2,

	// The magnetometer accuracy is poor and introduces unnecessary corrections to the orientation data.
	// Even after performing the figure-eight calibration, the accuracy does not improve,
	// so we will use Quat6 data instead.

	// To read Quat9, you must change num_commas == 5.
	bool readLine() {
		if (!initialized || hSerial == INVALID_HANDLE_VALUE) return false;

		DWORD bytesRead = 0;
		char receive_data_buffer[300] = { 0 };

		bool success = ReadFile(
			hSerial,
			receive_data_buffer,
			sizeof(receive_data_buffer),
			&bytesRead,
			NULL);

		if (!success || bytesRead == 0) return false;

		std::vector<char> read_vector(receive_data_buffer, receive_data_buffer + bytesRead);

		size_t j = 0;
		int z = 0;
		int num_comas = 0;
		double array[10] = { 0 };
		char auxiliar[300] = { 0 };
		bool correct = false;

		while (j < read_vector.size()) {
			char c = read_vector[j];

			if (c != '\r' && c != '\n') {
				auxiliar[z] = c;
				if (auxiliar[z] == ',') num_comas++;
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

						if (pEnd == token) {
							linea_correcta = false;
							break;
						}

						if (pos == 0 && array[pos] <= 1000) linea_correcta = false;

						token = strtok(NULL, ",");
						pos++;
					}

					if (linea_correcta) {
						float q1 = static_cast<float>(array[1]);
						float q2 = static_cast<float>(array[2]);
						float q3 = static_cast<float>(array[3]);

						if (linea_correcta) {

							float sum = q1 * q1 + q2 * q2 + q3 * q3;
							if (sum > 1.0f) sum = 1.0f;

							float q0 = sqrtf(1.0f - sum);

							CQuaternion qRaw = { q0, q1, q2, q3 };

							float norm = sqrtf(qRaw.w * qRaw.w + qRaw.x * qRaw.x + qRaw.y * qRaw.y + qRaw.z * qRaw.z);

							if (norm > 0.0001f) {
								qRaw.w /= norm;
								qRaw.x /= norm;
								qRaw.y /= norm;
								qRaw.z /= norm;
							} else {
								linea_correcta = false;
							}

							if (linea_correcta) {
								CQuaternion result = applyOffset(qRaw, trackerCenterOffset);

								if (std::isfinite(result.w) && std::isfinite(result.x) && std::isfinite(result.y) && std::isfinite(result.z)) {

									lastQuat = result;
									correct = true;
									break;
								}
							}
						}
					}
				}

				memset(auxiliar, 0, sizeof(auxiliar));
				z = 0;
				num_comas = 0;
			}

			j++;
		}

		return correct;
	}

	void calculateEulerAngles(double & yaw, double & pitch, double & roll) {
		double qw = lastQuat.w;
		double qx = lastQuat.x;
		double qy = lastQuat.y;
		double qz = lastQuat.z;

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


	CQuaternion getLastQuat() const { return lastQuat; }


	static std::vector<COMInfo> listCOMPorts() {
		std::vector<COMInfo> coms;

		HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT);
		if (hDevInfo == INVALID_HANDLE_VALUE) return coms;

		SP_DEVINFO_DATA devInfoData;
		devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); i++) {
			char COMName[256] = { 0 };
			if (SetupDiGetDeviceRegistryPropertyA(
					hDevInfo,
					&devInfoData,
					SPDRP_FRIENDLYNAME,
					NULL,
					(PBYTE)COMName,
					sizeof(COMName),
					NULL)) {
				std::string fname(COMName);
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

	// With the magnetometer enabled, the sensor is continuously self-calibrating.
	// Performing figure-8 movements helps cover all rotation axes (X, Y, Z),
	// improving the quality and stability of the calibration process.
	// Only usefull with quat9 line format 
	void calibrateSensor(int totalSamples) {

		if (!isConnected()) {
			std::cout << "Tracker not connected. Please connect first.\n";
			return;
		}

		std::cout << "=== IMU Sensor Calibration ===\n";
		std::cout << "Move the tracker in a slow figure-8 motion.\n";
		std::cout << "Cover all axes (X, Y, Z).\n";
		std::cout << "Press Enter to start...\n";
		std::cin.get();

		// Clear serial buffer before starting acquisition
		PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

		int samplesRead = 0;

		while (samplesRead < totalSamples) {
			cout << samplesRead << endl;

			if (readLine()) {
				samplesRead++;
			}
		}
	}


void calibrateCenter(int samplesPerDirection) {
		if (!isConnected()) {
			std::cout << "Tracker not connected. Please connect first.\n";
			return;
		}

		std::cout << "=== Tracker center calibration ===\n";
		std::cout << "Press Enter to start...\n";
		std::cin.get();

		PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

		CQuaternion motionPitch = { 0, 0, 0, 0 };
		int samplesRead = 0;

		std::cout << "Tilt your head up and down multiple times (\"yes\" motion)...\n";

		CQuaternion refPitch = { 0, 0, 0, 0 };
		bool firstPitch = true;

		while (samplesRead < samplesPerDirection) {

			if (readLine()) {

				if (!std::isfinite(lastQuat.w) || !std::isfinite(lastQuat.x) || !std::isfinite(lastQuat.y) || !std::isfinite(lastQuat.z)) {
					continue;
				}

				if (firstPitch) {
					refPitch = lastQuat;
					firstPitch = false;
				} else {
					float dot = lastQuat.w * refPitch.w + lastQuat.x * refPitch.x + lastQuat.y * refPitch.y + lastQuat.z * refPitch.z;

					if (dot < 0.0f) {
						lastQuat.w = -lastQuat.w;
						lastQuat.x = -lastQuat.x;
						lastQuat.y = -lastQuat.y;
						lastQuat.z = -lastQuat.z;
					}
				}

				motionPitch.w += lastQuat.w;
				motionPitch.x += lastQuat.x;
				motionPitch.y += lastQuat.y;
				motionPitch.z += lastQuat.z;

				samplesRead++;
			}
		}

		CQuaternion averagePitch = {
			motionPitch.w / samplesRead,
			motionPitch.x / samplesRead,
			motionPitch.y / samplesRead,
			motionPitch.z / samplesRead
		};

		float normPitch = sqrt(averagePitch.w * averagePitch.w + averagePitch.x * averagePitch.x + averagePitch.y * averagePitch.y + averagePitch.z * averagePitch.z);

		if (normPitch > 0.0001f) {
			averagePitch.w /= normPitch;
			averagePitch.x /= normPitch;
			averagePitch.y /= normPitch;
			averagePitch.z /= normPitch;
		}

		PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

		CQuaternion motionYaw = { 0, 0, 0, 0 };
		samplesRead = 0;

		std::cout << "Tilt your head side to side multiple times (\"no\" motion)...\n";

		CQuaternion refYaw = { 0, 0, 0, 0 };
		bool firstYaw = true;

		while (samplesRead < samplesPerDirection) {
			if (readLine()) {

				if (!std::isfinite(lastQuat.w) || !std::isfinite(lastQuat.x) || !std::isfinite(lastQuat.y) || !std::isfinite(lastQuat.z)) {
					continue;
				}

				if (firstYaw) {
					refYaw = lastQuat;
					firstYaw = false;
				} else {
					float dot = lastQuat.w * refYaw.w + lastQuat.x * refYaw.x + lastQuat.y * refYaw.y + lastQuat.z * refYaw.z;

					if (dot < 0.0f) {
						lastQuat.w = -lastQuat.w;
						lastQuat.x = -lastQuat.x;
						lastQuat.y = -lastQuat.y;
						lastQuat.z = -lastQuat.z;
					}
				}

				motionYaw.w += lastQuat.w;
				motionYaw.x += lastQuat.x;
				motionYaw.y += lastQuat.y;
				motionYaw.z += lastQuat.z;

				samplesRead++;
			}
		}

		CQuaternion averageYaw = {
			motionYaw.w / samplesRead,
			motionYaw.x / samplesRead,
			motionYaw.y / samplesRead,
			motionYaw.z / samplesRead
		};

		float normYaw = sqrt(averageYaw.w * averageYaw.w + averageYaw.x * averageYaw.x + averageYaw.y * averageYaw.y + averageYaw.z * averageYaw.z);

		if (normYaw > 0.0001f) {
			averageYaw.w /= normYaw;
			averageYaw.x /= normYaw;
			averageYaw.y /= normYaw;
			averageYaw.z /= normYaw;
		}

		// Compute center offset 
		trackerCenterOffset.w = (averagePitch.w + averageYaw.w) / 2.0f;
		trackerCenterOffset.x = (averagePitch.x + averageYaw.x) / 2.0f;
		trackerCenterOffset.y = (averagePitch.y + averageYaw.y) / 2.0f;
		trackerCenterOffset.z = (averagePitch.z + averageYaw.z) / 2.0f;

		// Normalize
		float norm = sqrt(
			trackerCenterOffset.w * trackerCenterOffset.w + trackerCenterOffset.x * trackerCenterOffset.x + trackerCenterOffset.y * trackerCenterOffset.y + trackerCenterOffset.z * trackerCenterOffset.z);

		if (norm > 0.0001f) {
			trackerCenterOffset.w /= norm;
			trackerCenterOffset.x /= norm;
			trackerCenterOffset.y /= norm;
			trackerCenterOffset.z /= norm;
		} else {
			std::cout << "WARNING: Invalid offset, resetting\n";
			trackerCenterOffset = { 1.0f, 0.0f, 0.0f, 0.0f };
		}

		std::cout << "Calibration completed. Center offset applied.\n";
		std::cout << "Offset: q0=" << trackerCenterOffset.w
				  << ", q1=" << trackerCenterOffset.x
				  << ", q2=" << trackerCenterOffset.y
				  << ", q3=" << trackerCenterOffset.z << "\n";
	}

private:
	HANDLE hSerial;
	bool initialized;
	CQuaternion lastQuat;
	CQuaternion trackerCenterOffset = { 1.0f, 0.0f, 0.0f, 0.0f };

	void PrintLastError(const char * functionName) {
		DWORD error = GetLastError();
		char message[256];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error, 0, message, sizeof(message), NULL);
		std::cout << functionName << " failed. Error " << error << ": " << message << std::endl;
	}

	CQuaternion applyOffset(const CQuaternion & q, const CQuaternion & offset) {
		CQuaternion offsetInv = { offset.w, -offset.x, -offset.y, -offset.z };
		CQuaternion result;
		result.w = q.w * offsetInv.w - q.x * offsetInv.x - q.y * offsetInv.y - q.z * offsetInv.z;
		result.x = q.w * offsetInv.x + q.x * offsetInv.w + q.y * offsetInv.z - q.z * offsetInv.y;
		result.y = q.w * offsetInv.y - q.x * offsetInv.z + q.y * offsetInv.w + q.z * offsetInv.x;
		result.z = q.w * offsetInv.z + q.x * offsetInv.y - q.y * offsetInv.x + q.z * offsetInv.w;
		return result;
	}
};

#endif
