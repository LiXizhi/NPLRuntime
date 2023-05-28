//-----------------------------------------------------------------------------
// Class: Serial Port
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2023.5.26
// Revised: 2023.5.26
// Notes: 
//-----------------------------------------------------------------------------

/*
 * File:   SerialPort.cpp
 * Author: Terraneo Federico
 * Distributed under the Boost Software License, Version 1.0.
 * Created on September 7, 2009, 10:46 AM
 */

#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "NPLTable.h"
#include "NPLHelper.h"
#include "INPLRuntimeState.h"

#include "SerialPort.h"

#include <string>
#include <algorithm>
#include <thread>
#include <mutex>
#include <boost/bind.hpp>
#include <boost/shared_array.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>

using namespace std;
using namespace boost;

namespace ParaEngine
{
    class SerialPortImpl : private boost::noncopyable
    {
    public:
        SerialPortImpl() : io(), port(io), backgroundThread(), open(false),
            error(false) {}

        boost::asio::io_service io; ///< Io service object
        boost::asio::serial_port port; ///< Serial port object
        std::thread backgroundThread; ///< Thread that runs read/write operations
        bool open; ///< True if port open
        bool error; ///< Error flag
        mutable std::mutex errorMutex; ///< Mutex for access to error

        /// Data are queued here before they go in writeBuffer
        std::vector<char> writeQueue;
        boost::shared_array<char> writeBuffer; ///< Data being written
        size_t writeBufferSize; ///< Size of writeBuffer
        std::mutex writeQueueMutex; ///< Mutex for access to writeQueue
        char readBuffer[SerialPort::readBufferSize]; ///< data being read

        /// Read complete callback
        std::function<void(const char*, size_t)> callback;
        std::string callbackScript;
        std::string sPortName;
    };

    SerialPort::SerialPort() : pimpl(new SerialPortImpl)
    {
    }

    SerialPort::SerialPort(const std::string& devname, unsigned int baud_rate,
        asio::serial_port_base::parity opt_parity,
        asio::serial_port_base::character_size opt_csize,
        asio::serial_port_base::flow_control opt_flow,
        asio::serial_port_base::stop_bits opt_stop)
        : pimpl(new SerialPortImpl)
    {
        open(devname, baud_rate, opt_parity, opt_csize, opt_flow, opt_stop);
    }

    void SerialPort::open(const std::string& devname, unsigned int baud_rate,
        asio::serial_port_base::parity opt_parity,
        asio::serial_port_base::character_size opt_csize,
        asio::serial_port_base::flow_control opt_flow,
        asio::serial_port_base::stop_bits opt_stop)
    {
        if (isOpen()) close();

        setErrorStatus(true);//If an exception is thrown, error_ remains true
        pimpl->sPortName = devname;
        pimpl->port.open(devname);
        pimpl->port.set_option(asio::serial_port_base::baud_rate(baud_rate));
        pimpl->port.set_option(opt_parity);
        pimpl->port.set_option(opt_csize);
        pimpl->port.set_option(opt_flow);
        pimpl->port.set_option(opt_stop);

        //This gives some work to the io_service before it is started
        pimpl->io.post(boost::bind(&SerialPort::doRead, this));

        std::thread t(boost::bind(&asio::io_service::run, &pimpl->io));
        pimpl->backgroundThread.swap(t);
        setErrorStatus(false);//If we get here, no error
        pimpl->open = true; //Port is now open
    }

    bool SerialPort::isOpen() const
    {
        return pimpl->open;
    }

    bool SerialPort::errorStatus() const
    {
        std::lock_guard<std::mutex> l(pimpl->errorMutex);
        return pimpl->error;
    }

    void SerialPort::close()
    {
        if (!isOpen()) return;

        pimpl->open = false;
        pimpl->io.post(boost::bind(&SerialPort::doClose, this));
        pimpl->backgroundThread.join();
        pimpl->io.reset();
        if (errorStatus())
        {
            throw(boost::system::system_error(boost::system::error_code(),
                "Error while closing the device"));
        }
    }

    void SerialPort::write(const char* data, size_t size)
    {
        {
            std::lock_guard<std::mutex> l(pimpl->writeQueueMutex);
            pimpl->writeQueue.insert(pimpl->writeQueue.end(), data, data + size);
        }
        pimpl->io.post(boost::bind(&SerialPort::doWrite, this));
    }

    void SerialPort::write(const std::vector<char>& data)
    {
        {
            std::lock_guard<std::mutex> l(pimpl->writeQueueMutex);
            pimpl->writeQueue.insert(pimpl->writeQueue.end(), data.begin(), data.end());
        }
        pimpl->io.post(boost::bind(&SerialPort::doWrite, this));
    }

    void SerialPort::writeString(const std::string& s)
    {
        {
            std::lock_guard<std::mutex> l(pimpl->writeQueueMutex);
            pimpl->writeQueue.insert(pimpl->writeQueue.end(), s.begin(), s.end());
        }
        pimpl->io.post(boost::bind(&SerialPort::doWrite, this));
    }

    SerialPort::~SerialPort()
    {
        if (isOpen())
        {
            try {
                close();
            }
            catch (...)
            {
                //Don't throw from a destructor
            }
        }
        clearReadCallback();
    }

    void SerialPort::doRead()
    {
        pimpl->port.async_read_some(asio::buffer(pimpl->readBuffer, readBufferSize),
            boost::bind(&SerialPort::readEnd,
                this,
                asio::placeholders::error,
                asio::placeholders::bytes_transferred));
    }

    void SerialPort::readEnd(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (error)
        {
            //error can be true even because the serial port was closed.
            //In this case it is not a real error, so ignore
            if (isOpen())
            {
                doClose();
                setErrorStatus(true);
            }
        }
        else 
        {
            if (pimpl->callback) 
                pimpl->callback(pimpl->readBuffer, bytes_transferred);
            if (!pimpl->callbackScript.empty())
            {
                NPL::NPLObjectProxy msg;
                msg["filename"] = pimpl->sPortName;
                msg["data"] = std::string(pimpl->readBuffer, bytes_transferred);

                std::string sMsg;
                NPL::NPLHelper::NPLTableToString("msg", msg, sMsg);
                ParaEngine::CGlobals::GetNPLRuntime()->GetMainRuntimeState()->Activate_async(pimpl->callbackScript, sMsg.c_str(), (int)sMsg.size());
            }
            doRead();
        }
    }

    void SerialPort::doWrite()
    {
        //If a write operation is already in progress, do nothing
        if (pimpl->writeBuffer == 0)
        {
            std::lock_guard<std::mutex> l(pimpl->writeQueueMutex);
            pimpl->writeBufferSize = pimpl->writeQueue.size();
            pimpl->writeBuffer.reset(new char[pimpl->writeQueue.size()]);
            copy(pimpl->writeQueue.begin(), pimpl->writeQueue.end(),
                pimpl->writeBuffer.get());
            pimpl->writeQueue.clear();
            async_write(pimpl->port, asio::buffer(pimpl->writeBuffer.get(),
                pimpl->writeBufferSize),
                boost::bind(&SerialPort::writeEnd, this, asio::placeholders::error));
        }
    }

    void SerialPort::writeEnd(const boost::system::error_code& error)
    {
        if (!error)
        {
            std::lock_guard<std::mutex> l(pimpl->writeQueueMutex);
            if (pimpl->writeQueue.empty())
            {
                pimpl->writeBuffer.reset();
                pimpl->writeBufferSize = 0;

                return;
            }
            pimpl->writeBufferSize = pimpl->writeQueue.size();
            pimpl->writeBuffer.reset(new char[pimpl->writeQueue.size()]);
            copy(pimpl->writeQueue.begin(), pimpl->writeQueue.end(),
                pimpl->writeBuffer.get());
            pimpl->writeQueue.clear();
            async_write(pimpl->port, asio::buffer(pimpl->writeBuffer.get(),
                pimpl->writeBufferSize),
                boost::bind(&SerialPort::writeEnd, this, asio::placeholders::error));
        }
        else {
            setErrorStatus(true);
            doClose();
        }
    }

    void SerialPort::doClose()
    {
        boost::system::error_code ec;
        pimpl->port.cancel(ec);
        if (ec) setErrorStatus(true);
        pimpl->port.close(ec);
        if (ec) setErrorStatus(true);
    }

    void SerialPort::setErrorStatus(bool e)
    {
        std::lock_guard<std::mutex> l(pimpl->errorMutex);
        pimpl->error = e;
    }

    void SerialPort::setReadCallback(const std::function<void(const char*, size_t)>& callback)
    {
        pimpl->callback = callback;
    }

    void SerialPort::clearReadCallback()
    {
        std::function<void(const char*, size_t)> empty;
        pimpl->callback.swap(empty);
    }
    
    void SerialPort::setCallback(const std::function<void(const char*, size_t)>& callback)
    {
        setReadCallback(callback);
    }

    void SerialPort::setCallback(const std::string& callback)
    {
        pimpl->callbackScript = callback;
    }

    void SerialPort::clearCallback()
    {
        clearReadCallback();
    }
        
    /// <summary>
    /// global singleton for serial class
    /// </summary>
    Serial::Serial()
    {
    }

    Serial::~Serial()
    {
        for (auto serialport : m_allSerialPorts)
        {
			delete serialport.second;
		}
    }

    std::vector<std::string> Serial::GetPortNames()
    {
        std::vector<std::string> ports;
        // get all serial ports using win32
#ifdef WIN32
        char lpTargetPath[5000]; // buffer to store the path of the COMPORTS
        
        for (int i = 0; i < 99; i++) // checking ports from COM0 to COM99 or COM255
        {
            std::string str = "COM" + std::to_string(i); // converting to COM0, COM1, COM2
            DWORD test = QueryDosDevice(str.c_str(), lpTargetPath, 5000);
            if (test != 0) //QueryDosDevice returns zero if it didn't find an object
            {
                ports.push_back(std::string(str + ":" + lpTargetPath));
            }

            if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
            }
        }
#else
        // for linux / MAC, we need to use `ls /dev/tty*` to get all serial ports
        boost::filesystem::path kdr_path{"/proc/tty/drivers"};
        if (boost::filesystem::exists(kdr_path))
        {
            std::ifstream ifile(kdr_path.generic_string());
            std::string line;
            std::vector<std::string> prefixes;
            while (std::getline(ifile, line))
            {
                std::vector<std::string> items;
                auto it = line.find_first_not_of(' ');
                while (it != std::string::npos)
                {

                    auto it2 = line.substr(it).find_first_of(' ');
                    if (it2 == std::string::npos)
                    {
                        items.push_back(line.substr(it));
                        break;
                    }
                    it2 += it;
                    items.push_back(line.substr(it, it2 - it));
                    it = it2 + line.substr(it2).find_first_not_of(' ');
                }
                if (items.size() >= 5)
                {
                    if (items[4] == "serial" && items[0].find("serial") != std::string::npos)
                    {
                        prefixes.emplace_back(items[1]);
                    }
                }
            }
            ifile.close();
            for (auto& p: boost::filesystem::directory_iterator("/dev"))
            {
                for (const auto& pf : prefixes)
                {
                    auto dev_path = p.path().generic_string();
                    if (dev_path.size() >= pf.size() && std::equal(dev_path.begin(), dev_path.begin() + pf.size(), pf.begin()))
                    {
                        ports.emplace_back(dev_path);
                    }
                }
            }
        }
#endif
        return ports;
    }

    void Serial::AddSerialPort(const std::string& name, SerialPort* port)
    {
        RemoveSerialPort(name);
        m_allSerialPorts[name] = port;
    }

    void Serial::RemoveSerialPort(const std::string& name)
    {
        if (m_allSerialPorts.find(name) != m_allSerialPorts.end())
        {
            delete m_allSerialPorts[name];
            m_allSerialPorts.erase(name);
        }
    }

    SerialPort* Serial::GetSerialPort(const std::string& name)
    {
        if (m_allSerialPorts.find(name) != m_allSerialPorts.end())
        {
			return m_allSerialPorts[name];
		}
		return nullptr;
    }

    Serial* Serial::GetSingleton()
    {
        static Serial g_singleton;
        return &g_singleton;
    }
}


extern "C"
{
    /*  NPL.activate("script/serialport.cpp", {cmd="open|close|GetPortNames", filename="COM1", baud_rate=115200})
    */
    PE_CORE_DECL NPL::NPLReturnCode NPL_activate_script_serialport_cpp(NPL::INPLRuntimeState* pState)
    {
        auto msg = NPL::NPLHelper::MsgStringToNPLTable(pState->GetCurrentMsg(), pState->GetCurrentMsgLength());

        try
        {
            std::string cmd = msg["cmd"];
            if (cmd == "open")
            {
                std::string filename = msg["filename"];
                if (!filename.empty())
                {
                    // remove old if any
                    ParaEngine::Serial::GetSingleton()->RemoveSerialPort(filename);

                    // start a new one instead. 
                    auto pSerialPort = new ParaEngine::SerialPort();
                    ParaEngine::Serial::GetSingleton()->AddSerialPort(filename, pSerialPort);
                    int baud_rate = (int)(double)msg["baud_rate"];
                    if (baud_rate == 0) {
                        baud_rate = 115200;
                    }
                    std::string sCallback = msg["callback"];
                    if (!sCallback.empty())
                    {
                        pSerialPort->setCallback(sCallback);
                    }
                    pSerialPort->open(filename, baud_rate);
                }
            }
            else if (cmd == "send")
            {
                std::string filename = msg["filename"];
                if (!filename.empty())
                {
                    std::string data = msg["data"];
                    if (!data.empty())
                    {
                        auto* pPort = ParaEngine::Serial::GetSingleton()->GetSerialPort(filename);
                        if (pPort && pPort->isOpen())
                        {
                            pPort->writeString(data);
                        }
                    }
				}
            }
            else if (cmd == "close")
            {
                std::string filename = msg["filename"];
                if (!filename.empty())
                {
                    ParaEngine::Serial::GetSingleton()->RemoveSerialPort(filename);
                }
            }
            else if (cmd == "GetPortNames")
            {
                std::string sCallback = msg["callback"];
                if (!sCallback.empty())
                {
                    auto names = ParaEngine::Serial::GetSingleton()->GetPortNames();
                    std::string sNames = "msg={";
                    for (auto name : names)
                    {
                        std::string sOutput;
                        NPL::NPLHelper::EncodeStringInQuotation(sOutput, 0, name);
						sNames += sOutput + ",";
					}
                    sNames += "}";
                    ParaEngine::CGlobals::GetNPLRuntime()->GetMainRuntimeState()->ActivateFile(sCallback, sNames.c_str(), (int)sNames.size());
                }
            }
        }
        catch (...)
        {
            std::string sCallback = msg["callback"];
            if (!sCallback.empty())
            {
                msg["error"] = true;
                std::string callbackMsg;
                NPL::NPLHelper::NPLTableToString("msg", msg, callbackMsg);
                ParaEngine::CGlobals::GetNPLRuntime()->GetMainRuntimeState()->ActivateFile(sCallback, callbackMsg.c_str(), (int)callbackMsg.size());
            }
        }
        return NPL::NPL_OK;
    };
}
