//-----------------------------------------------------------------------------
// Class: Blue Tooth
// Authors:	big
// Emails:	onedous@gmail.com
// Date:	2023.12.18
// Notes: 
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "NPLTable.h"
#include "NPLHelper.h"
#include "INPLRuntimeState.h"

#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>
#include <roapi.h>
#include <condition_variable>
#include <map>
#include <mutex>
#pragma comment(lib, "windowsapp.lib")
#endif

#include "BlueTooth.h"

#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
namespace
{
	using namespace winrt::Windows::Devices::Bluetooth;
	using namespace winrt::Windows::Devices::Bluetooth::Advertisement;
	using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;

	struct Advertiser {
		std::string name;
		BluetoothAddressType type = BluetoothAddressType::Unspecified;
		int16_t rssi = 127; // HCI sentinel for an unavailable RSSI.
	};

	struct AdvertisementResults
	{
		std::mutex mutex;
		std::condition_variable stopped;
		bool ended = false;
		BluetoothError error = BluetoothError::Success;
		std::map<uint64_t, Advertiser> devices;
	};

	void CollectAdvertisements(NPL::NPLObjectProxy& result, int scanSeconds)
	{
		auto scan = std::make_shared<AdvertisementResults>();
		BluetoothLEAdvertisementWatcher watcher;
		watcher.ScanningMode(BluetoothLEScanningMode::Active);
		auto received = watcher.Received(winrt::auto_revoke, [scan](auto const&, BluetoothLEAdvertisementReceivedEventArgs const& event) {
			auto name = winrt::to_string(event.Advertisement().LocalName());
			std::lock_guard<std::mutex> lock(scan->mutex);
			auto& device = scan->devices[event.BluetoothAddress()];
			if (!name.empty()) device.name = std::move(name);
			const auto rssi = event.RawSignalStrengthInDBm();
			if (rssi >= -127 && rssi <= 20) device.rssi = rssi;
			if (auto info = event.try_as<IBluetoothLEAdvertisementReceivedEventArgs2>())
				device.type = info.BluetoothAddressType();
		});
		auto stopped = watcher.Stopped(winrt::auto_revoke, [scan](auto const&, BluetoothLEAdvertisementWatcherStoppedEventArgs const& event) {
			std::lock_guard<std::mutex> lock(scan->mutex);
			scan->error = event.Error();
			scan->ended = true;
			scan->stopped.notify_one();
		});
		watcher.Start();
		{
			std::unique_lock<std::mutex> lock(scan->mutex);
			const auto duration = std::chrono::seconds(scanSeconds >= 1 && scanSeconds <= 30 ? scanSeconds : 6);
			scan->stopped.wait_for(lock, duration, [&] { return scan->ended; });
		}
		watcher.Stop();
		std::lock_guard<std::mutex> lock(scan->mutex);
		if (scan->error != BluetoothError::Success || watcher.Status() == BluetoothLEAdvertisementWatcherStatus::Aborted)
		{
			result["status"] = "unavailable";
			return;
		}
		int index = 1;
		for (const auto& entry : scan->devices)
		{
			char address[18];
			snprintf(address, sizeof(address), "%02X:%02X:%02X:%02X:%02X:%02X",
				(unsigned)(entry.first >> 40) & 255, (unsigned)(entry.first >> 32) & 255,
				(unsigned)(entry.first >> 24) & 255, (unsigned)(entry.first >> 16) & 255,
				(unsigned)(entry.first >> 8) & 255, (unsigned)entry.first & 255);
			auto& device = result["devices"][index++];
			device["id"] = std::string(address);
			device["name"] = entry.second.name;
			device["addressType"] = (double)entry.second.type;
			if (entry.second.rssi != 127) device["rssi"] = (double)entry.second.rssi;
		}
		result["status"] = "ok";
	}

	std::string UuidText(winrt::guid uuid)
	{
		auto text = winrt::to_string(winrt::to_hstring(uuid));
		return !text.empty() && text.front() == '{' ? text.substr(1, text.size() - 2) : text;
	}

	uint64_t ParseAddress(const std::string& text)
	{
		if (text.size() != 17) throw winrt::hresult_invalid_argument();
		uint64_t address = 0;
		for (size_t i = 0; i < text.size(); ++i) {
			const char c = text[i];
			if (i % 3 == 2 && c == ':') continue;
			if (i % 3 == 2 || !std::isxdigit(static_cast<unsigned char>(c))) throw winrt::hresult_invalid_argument();
			address = (address << 4) | (c <= '9' ? c - '0' : (c | 32) - 'a' + 10);
		}
		return address;
	}

	template<class Operation>
	auto AwaitGatt(Operation op, std::chrono::steady_clock::time_point deadline)
	{
		using winrt::Windows::Foundation::AsyncStatus;
		auto remaining = deadline - std::chrono::steady_clock::now();
		auto span = std::chrono::duration_cast<winrt::Windows::Foundation::TimeSpan>(remaining);
		if (span.count() <= 0 || op.wait_for(span) == AsyncStatus::Started) {
			op.Cancel();
			throw winrt::hresult_error(HRESULT_FROM_WIN32(ERROR_TIMEOUT));
		}
		return op.GetResults();
	}

	// All commands and handle releases run on the same NPL worker apartment.
	struct GattClient
	{
		HRESULT apartment = RoInitialize(RO_INIT_MULTITHREADED);
		BluetoothLEDevice device{nullptr};
		using Endpoint = std::pair<std::string, std::string>;
		std::map<Endpoint, GattCharacteristic> characteristics;
		std::map<Endpoint, GattCharacteristic::ValueChanged_revoker> subscriptions;
		double session = 0;

		void Close()
		{
			session = 0;
			subscriptions.clear();
			characteristics.clear();
			if (device) {
				auto closing = std::move(device);
				closing.Close();
			}
		}

		~GattClient()
		{
			try { Close(); } catch (const winrt::hresult_error&) {}
			if (SUCCEEDED(apartment)) RoUninitialize();
		}

		void Connect(NPL::NPLObjectProxy& msg, NPL::NPLObjectProxy& result)
		{
			Close();
			if (FAILED(apartment) && apartment != RPC_E_CHANGED_MODE) winrt::check_hresult(apartment);
			const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(23);
			const auto address = ParseAddress(msg["id"]);
			const auto type = static_cast<BluetoothAddressType>(msg["addressType"].toInt());
			auto opening = type == BluetoothAddressType::Unspecified
				? BluetoothLEDevice::FromBluetoothAddressAsync(address)
				: BluetoothLEDevice::FromBluetoothAddressAsync(address, type);
			auto candidate = AwaitGatt(opening, deadline);
			if (!candidate) { result["status"] = "unavailable"; return; }
			auto services = AwaitGatt(candidate.GetGattServicesAsync(BluetoothCacheMode::Uncached), deadline);
			if (services.Status() != GattCommunicationStatus::Success) {
				result["status"] = services.Status() == GattCommunicationStatus::AccessDenied ? "denied" : "unavailable";
				candidate.Close();
				return;
			}
			int index = 1;
			for (const auto& service : services.Services()) {
				auto& row = result["services"][index++];
				row["uuid"] = UuidText(service.Uuid());
				try {
					auto chars = AwaitGatt(service.GetCharacteristicsAsync(BluetoothCacheMode::Cached), deadline);
					if (chars.Status() != GattCommunicationStatus::Success) row["partial"] = true;
					else {
						int j = 1;
						for (const auto& ch : chars.Characteristics()) {
							auto& item = row["characteristics"][j++];
							item["uuid"] = UuidText(ch.Uuid());
							item["properties"] = (double)ch.CharacteristicProperties();
							characteristics.emplace(Endpoint{UuidText(service.Uuid()), UuidText(ch.Uuid())}, ch);
						}
					}
				} catch (const winrt::hresult_error&) { row["partial"] = true; }
			}
			if (candidate.ConnectionStatus() != BluetoothConnectionStatus::Connected) {
				candidate.Close();
				result["status"] = "disconnected";
				return;
			}
			device = std::move(candidate);
			session = (double)msg["request"];
			result["status"] = "connected";
		}

		bool Subscribe(const Endpoint& key, std::chrono::steady_clock::time_point deadline)
		{
			if (subscriptions.count(key)) return true;
			auto found = characteristics.find(key);
			if (found == characteristics.end()) return false;
			const auto& ch = found->second;
			const auto props = ch.CharacteristicProperties();
			auto mode = GattClientCharacteristicConfigurationDescriptorValue::Notify;
			if ((props & GattCharacteristicProperties::Notify) == GattCharacteristicProperties::None) {
				if ((props & GattCharacteristicProperties::Indicate) == GattCharacteristicProperties::None) return false;
				mode = GattClientCharacteristicConfigurationDescriptorValue::Indicate;
			}
			auto mainState = NPL::CNPLRuntime::GetInstance()->GetMainRuntimeState();
			auto listener = ch.ValueChanged(winrt::auto_revoke, [key, request = session, mainState](auto const&, GattValueChangedEventArgs const& args) {
				auto reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(args.CharacteristicValue());
				std::string hex;
				const char digits[] = "0123456789abcdef";
				while (reader.UnconsumedBufferLength()) {
					auto byte = reader.ReadByte();
					hex.push_back(digits[byte >> 4]);
					hex.push_back(digits[byte & 15]);
				}
				NPL::NPLObjectProxy event;
				event["channel"] = "gatt";
				event["status"] = "notification";
				event["request"] = request;
				event["service"] = key.first;
				event["characteristic"] = key.second;
				event["hex"] = hex;
				std::string reply;
				NPL::NPLHelper::NPLTableToString("msg", event, reply);
				mainState->Activate_async("script/ide/System/os/BluetoothDiscovery.lua", reply.c_str(), (int)reply.size());
			});
			if (AwaitGatt(ch.WriteClientCharacteristicConfigurationDescriptorAsync(mode), deadline) != GattCommunicationStatus::Success) return false;
			subscriptions.emplace(key, std::move(listener));
			return true;
		}

		bool Write(NPL::NPLObjectProxy& msg)
		{
			if (!device || session != (double)msg["request"] || device.ConnectionStatus() != BluetoothConnectionStatus::Connected)
				return false;
			const std::string service = msg["service"], characteristic = msg["characteristic"], hex = msg["hex"];
			auto target = characteristics.find({service, characteristic});
			if (target == characteristics.end() || hex.empty() || hex.size() > 1024 || hex.size() % 2) return false;
			winrt::Windows::Storage::Streams::DataWriter writer;
			for (size_t i = 0; i < hex.size(); i += 2) {
				unsigned byte = 0;
				for (size_t j = i; j < i + 2; ++j) {
					const unsigned char c = hex[j];
					if (!std::isxdigit(c)) return false;
					byte = byte * 16 + (c <= '9' ? c - '0' : (c | 32) - 'a' + 10);
				}
				writer.WriteByte(static_cast<uint8_t>(byte));
			}
			auto mode = GattWriteOption::WriteWithResponse;
			if ((target->second.CharacteristicProperties() & GattCharacteristicProperties::Write) == GattCharacteristicProperties::None) {
				if ((target->second.CharacteristicProperties() & GattCharacteristicProperties::WriteWithoutResponse) == GattCharacteristicProperties::None) return false;
				if (msg["allowWithoutResponse"].GetType() != NPL::NPLObjectBase::NPLObjectType_Bool || !(bool)msg["allowWithoutResponse"]) return false;
				mode = GattWriteOption::WriteWithoutResponse;
			}
			const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
			if (msg["notifyCharacteristic"].GetType() == NPL::NPLObjectBase::NPLObjectType_String &&
				!Subscribe({service, (std::string)msg["notifyCharacteristic"]}, deadline)) return false;
			auto op = target->second.WriteValueAsync(writer.DetachBuffer(), mode);
			return AwaitGatt(op, deadline) == GattCommunicationStatus::Success;
		}
	};
}
#endif

namespace ParaEngine
{
	static BlueTooth* g_bluetooth = nullptr;

	BlueTooth::BlueTooth()
	{
	}

	BlueTooth::~BlueTooth()
	{
	}

	BlueTooth* BlueTooth::GetSingleton()
	{
		if (g_bluetooth == nullptr)
		{
			g_bluetooth = new BlueTooth();
		}
		return g_bluetooth;
	}

	void BlueTooth::setDeviceName()
	{
	}

	void BlueTooth::setupBluetoothDelegate()
	{
	}

	void BlueTooth::readAllBlueGatt()
	{
	}

	void BlueTooth::disconnectBlueTooth()
	{
	}

	void BlueTooth::linkDevice()
	{
	}

	void BlueTooth::writeToCharacteristic()
	{
	}

	void BlueTooth::readCharacteristic()
	{
	}

	void BlueTooth::setCharacteristicNotification()
	{
	}

	void BlueTooth::setDescriptorNotification()
	{
	}
}

extern "C"
{
	PE_CORE_DECL NPL::NPLReturnCode NPL_activate_script_bluetooth_cpp(NPL::INPLRuntimeState* pState)
	{
		NPL::NPLObjectProxy msg = NPL::NPLHelper::MsgStringToNPLTable(pState->GetCurrentMsg(), pState->GetCurrentMsgLength());

		try
		{
			if (msg["cmd"].GetType() == NPL::NPLObjectBase::NPLObjectType_String)
			{
				std::string cmd = msg["cmd"];

				if (cmd == "discover")
				{
					// The timed scan runs only on the dedicated worker, never on the game thread.
					if (pState->GetName() != "BluetoothDiscovery") return NPL::NPL_OK;
					NPL::NPLObjectProxy result;
					result["request"] = (double)msg["request"];
					result["status"] = "unsupported";
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
					HRESULT apartment = RoInitialize(RO_INIT_MULTITHREADED);
					try
					{
						if (FAILED(apartment) && apartment != RPC_E_CHANGED_MODE) winrt::check_hresult(apartment);
						CollectAdvertisements(result, msg["scanSeconds"].toInt());
					}
					catch (const winrt::hresult_error&)
					{
						result["status"] = "unavailable";
					}
					if (SUCCEEDED(apartment)) RoUninitialize();
#endif
					std::string reply;
					NPL::NPLHelper::NPLTableToString("msg", result, reply);
					pState->activate("(main)script/ide/System/os/BluetoothDiscovery.lua", reply.c_str(), (int)reply.size());
				}
				else if (cmd == "gattConnect" || cmd == "gattDisconnect" || cmd == "gattStatus" || cmd == "gattWrite")
				{
					if (pState->GetName() != "BluetoothGatt") return NPL::NPL_OK;
					NPL::NPLObjectProxy result;
					result["request"] = (double)msg["request"];
					result["channel"] = "gatt";
					if (cmd == "gattWrite") result["operation"] = (double)msg["operation"];
					result["status"] = "unsupported";
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
					thread_local GattClient client;
					try {
						if (cmd == "gattConnect") client.Connect(msg, result);
						else if (cmd == "gattWrite") result["status"] = client.Write(msg) ? "written" : "writeFailed";
						else if (client.session == (double)msg["request"] && client.device &&
							cmd == "gattStatus" && client.device.ConnectionStatus() == BluetoothConnectionStatus::Connected)
							result["status"] = "alive";
						else {
							if (client.session == (double)msg["request"]) client.Close();
							result["status"] = "disconnected";
						}
					} catch (const winrt::hresult_error& ex) {
						if (cmd != "gattWrite") client.Close();
						result["status"] = ex.code() == E_ACCESSDENIED ? "denied" :
							ex.code() == HRESULT_FROM_WIN32(ERROR_TIMEOUT) ? "timeout" : "unavailable";
					}
#endif
					std::string reply;
					NPL::NPLHelper::NPLTableToString("msg", result, reply);
					pState->activate("(main)script/ide/System/os/BluetoothDiscovery.lua", reply.c_str(), (int)reply.size());
				}
				else if (cmd == "setDeviceName")
				{
					NPL::NPLObjectProxy args = msg["args"];
					if (args["name"].GetType() == NPL::NPLObjectBase::NPLObjectType_String)
					{
						std::string name = args["name"];
						OUTPUT_LOG("%s\n", name.c_str());
					}
				}
				else if (cmd == "setupBluetoothDelegate")
				{
				}
				else if (cmd == "readAllBlueGatt")
				{
				}
				else if (cmd == "disconnectBlueTooth")
				{
				}
				else if (cmd == "linkDevice")
				{
				}
				else if (cmd == "writeToCharacteristic")
				{
				}
				else if (cmd == "readCharacteristic")
				{
				}
				else if (cmd == "setCharacteristicNotification")
				{
				}
				else if (cmd == "setDescriptorNotification")
				{
				}
			}
		}
		catch (...)
		{
		}

		return NPL::NPL_OK;
	}
}
