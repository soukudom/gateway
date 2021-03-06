#pragma once

#include <map>
#include <vector>

#include <Poco/Mutex.h>
#include <Poco/Timespan.h>

#include "core/AbstractSeeker.h"
#include "core/DeviceManager.h"
#include "core/PollingKeeper.h"
#include "credentials/FileCredentialsStorage.h"
#include "loop/StopControl.h"
#include "model/DeviceID.h"
#include "model/RefreshTime.h"
#include "net/MACAddress.h"
#include "philips/PhilipsHueBridge.h"
#include "philips/PhilipsHueBulb.h"
#include "philips/PhilipsHueListener.h"
#include "util/AsyncWork.h"
#include "util/CryptoConfig.h"
#include "util/EventSource.h"

namespace BeeeOn {

/**
 * @brief The class implements the work with Philips Hue bulbs. Allows us
 * to process and execute the commands from server. It means modify state
 * of proper bulb.
 */
class PhilipsHueDeviceManager : public DeviceManager {
public:
	/**
	 * @brief Provides searching philips devices on network
	 * in own thread.
	 */
	class PhilipsHueSeeker : public AbstractSeeker {
	public:
		typedef Poco::SharedPtr<PhilipsHueSeeker> Ptr;

		PhilipsHueSeeker(PhilipsHueDeviceManager& parent, const Poco::Timespan& duration);

	protected:
		void seekLoop(StopControl &control) override;

	private:
		PhilipsHueDeviceManager& m_parent;
	};

	static const Poco::Timespan SEARCH_DELAY;

	PhilipsHueDeviceManager();

	void run() override;
	void stop() override;

	void setDevicePoller(DevicePoller::Ptr poller);
	void setUPnPTimeout(const Poco::Timespan &timeout);
	void setHTTPTimeout(const Poco::Timespan &timeout);
	void setRefresh(const Poco::Timespan &refresh);
	void setCredentialsStorage(Poco::SharedPtr<FileCredentialsStorage> storage);
	void setCryptoConfig(Poco::SharedPtr<CryptoConfig> config);
	void setEventsExecutor(AsyncExecutor::Ptr executor);
	void registerListener(PhilipsHueListener::Ptr listener);

protected:
	void handleAccept(const DeviceAcceptCommand::Ptr cmd) override;
	AsyncWork<>::Ptr startDiscovery(const Poco::Timespan &timeout) override;
	AsyncWork<std::set<DeviceID>>::Ptr startUnpair(
		const DeviceID &id,
		const Poco::Timespan &timeout) override;
	AsyncWork<double>::Ptr startSetValue(
		const DeviceID &id,
		const ModuleID &module,
		const double value,
		const Poco::Timespan &timeout) override;

	void searchPairedDevices();

	/**
	 * @brief Erases the bridges which don't care any bulb.
	 */
	void eraseUnusedBridges();

	std::vector<PhilipsHueBulb::Ptr> seekBulbs(const StopControl& stop);

	void authorizationOfBridge(PhilipsHueBridge::Ptr bridge);

	void processNewDevice(PhilipsHueBulb::Ptr newDevice);

	void fireBridgeStatistics(PhilipsHueBridge::Ptr bridge);
	void fireBulbStatistics(PhilipsHueBulb::Ptr bulb);

private:
	Poco::FastMutex m_bridgesMutex;
	Poco::FastMutex m_pairedMutex;

	std::map<MACAddress, PhilipsHueBridge::Ptr> m_bridges;
	std::map<DeviceID, PhilipsHueBulb::Ptr> m_devices;

	PollingKeeper m_pollingKeeper;
	RefreshTime m_refresh;
	Poco::Timespan m_httpTimeout;
	Poco::Timespan m_upnpTimeout;

	Poco::SharedPtr<FileCredentialsStorage> m_credentialsStorage;
	Poco::SharedPtr<CryptoConfig> m_cryptoConfig;

	EventSource<PhilipsHueListener> m_eventSource;
};

}
