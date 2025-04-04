#include "utilityApp.h"

#include <algorithm>
#include <mutex>
#include <set>

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read.hpp>
#include <boost/chrono.hpp>
#if BOOST_VERSION >= 108600
	#include <boost/process/v1.hpp>
#else
	#include <boost/process.hpp>
#endif
#include <boost/thread.hpp>

#include <QThread>

#include "ScopedFunctor.h"
#include "logging.h"
#include "utilityString.h"

using namespace boost;
using namespace boost::chrono;

namespace utility
{
std::mutex s_runningProcessesMutex;
std::set<std::shared_ptr<boost::process::child>> s_runningProcesses;

std::string getDocumentationLink()
{
	return "https://github.com/petermost/Sourcetrail/blob/master/DOCUMENTATION.md";
}

std::wstring searchPath(const std::wstring& bin, bool& ok)
{
	ok = false;
	std::wstring r = boost::process::search_path(bin).generic_wstring();
	if (!r.empty())
	{
		ok = true;
		return r;
	}
	return bin;
}

std::wstring searchPath(const std::wstring& bin)
{
	bool ok;
	return searchPath(bin, ok);
}

namespace
{
bool wait_for_process(boost::process::child *process, milliseconds timeout)
{
	// This function used to call 'process::child::wait_for()' which issued the warning "wait_for is unreliable".
	// See these tickets for further information:
	// https://github.com/klemens-morgenstern/boost-process/issues/99
	// https://github.com/klemens-morgenstern/boost-process/issues/112

	constexpr milliseconds POLL_INTERVAL(100);

	while (process->running() && timeout > milliseconds::zero())
	{
		this_thread::sleep_for(POLL_INTERVAL);
		timeout -= std::min(timeout, POLL_INTERVAL);
	}
	return process->running();
}
}	 // namespace

ProcessOutput executeProcess(const std::wstring& command, const std::vector<std::wstring>& arguments, const FilePath& workingDirectory,
	const bool waitUntilNoOutput, const milliseconds &timeout, bool logProcessOutput)
{
	std::string output;
	int exitCode = 255;
	try
	{
		boost::asio::io_context ctx;
		boost::process::async_pipe ap(ctx);

		std::shared_ptr<boost::process::child> process;

		boost::process::environment env = boost::this_process::environment();
		std::vector<std::string> previousPath = env["PATH"].to_vector();
		env["PATH"] = {"/opt/local/bin", "/usr/local/bin", "$HOME/bin"};
		for (const std::string& entry: previousPath)
		{
			env["PATH"].append(entry);
		}

		if (workingDirectory.empty())
		{
			process = std::make_shared<boost::process::child>(
				searchPath(command),
				boost::process::args(arguments),
				env,
				boost::process::std_in.close(),
				(boost::process::std_out & boost::process::std_err) > ap);
		}
		else
		{
			process = std::make_shared<boost::process::child>(
				searchPath(command),
				boost::process::args(arguments),
				boost::process::start_dir(workingDirectory.wstr()),
				env,
				boost::process::std_in.close(),
				(boost::process::std_out & boost::process::std_err) > ap);
		}

		{
			std::lock_guard<std::mutex> lock(s_runningProcessesMutex);
			s_runningProcesses.insert(process);
		}

		ScopedFunctor remover(
			[process]()
			{
				std::lock_guard<std::mutex> lock(s_runningProcessesMutex);
				s_runningProcesses.erase(process);
			});

		bool outputReceived = false;
		std::vector<char> buf(128);
		auto stdOutBuffer = boost::asio::buffer(buf);
		std::string logBuffer;

		std::function<void(const boost::system::error_code& ec, std::size_t n)> onStdOut =
			[&output, &buf, &stdOutBuffer, &ap, &onStdOut, &outputReceived, &logBuffer, logProcessOutput](
				const boost::system::error_code& ec, std::size_t size)
		{
			std::string text;
			text.reserve(size);
			text.insert(text.end(), buf.begin(), buf.begin() + size);

			if (!text.empty())
			{
				outputReceived = true;
			}

			output += text;
			if (logProcessOutput)
			{
				logBuffer += text;
				const bool isEndOfLine = (logBuffer.back() == '\n');
				const std::vector<std::string> lines = splitToVector(logBuffer, "\n");
				for (size_t i = 0; i < lines.size() - (isEndOfLine ? 0 : 1); i++)
				{
					LOG_INFO_BARE("Process output: " + lines[i]);
				}
				if (isEndOfLine)
				{
					logBuffer.clear();
				}
				else
				{
					logBuffer = lines.back();
				}
			}
			if (!ec)
			{
				boost::asio::async_read(ap, stdOutBuffer, onStdOut);
			}
		};

		boost::asio::async_read(ap, stdOutBuffer, onStdOut);
		ctx.run();

		if (timeout != INFINITE_TIMEOUT)
		{
			if (waitUntilNoOutput)
			{
				while (!wait_for_process(process.get(), timeout))
				{
					if (!outputReceived)
					{
						LOG_WARNING("Canceling process because it did not generate any output during the "
							"last " + boost::chrono::to_string(duration_cast<seconds>(timeout)) + " seconds.");
						process->terminate();
						break;
					}
					outputReceived = false;
				}
			}
			else
			{
				if (!wait_for_process(process.get(), timeout))
				{
					LOG_WARNING("Canceling process because it timed out after " +
						boost::chrono::to_string(duration_cast<seconds>(timeout)) + " seconds.");
					process->terminate();
				}
			}
		}
		else
		{
			process->wait();
		}

		if (logProcessOutput)
		{
			for (const std::string& line: splitToVector(logBuffer, "\n"))
			{
				LOG_INFO_BARE("Process output: " + line);
			}
		}

		exitCode = process->exit_code();
	}
	catch (const boost::process::process_error& e)
	{
		ProcessOutput ret;
		ret.error = decodeFromUtf8(e.code().message());
		ret.exitCode = e.code().value();
		LOG_ERROR_BARE(L"Process error: " + ret.error);

		return ret;
	}

	ProcessOutput ret;
	ret.output = trim(decodeFromUtf8(output));
	ret.exitCode = exitCode;
	return ret;
}

void killRunningProcesses()
{
	std::lock_guard<std::mutex> lock(s_runningProcessesMutex);
	for (std::shared_ptr<boost::process::child> process: s_runningProcesses)
	{
		process->terminate();
	}
}

int getIdealThreadCount()
{
	int threadCount = QThread::idealThreadCount();
	if constexpr (Platform::isWindows())
	{
		threadCount -= 1;
	}
	return std::max(1, threadCount);
}

}	 // namespace utility

/* Not referenced anywhere!
enum class OsType
{
	UNKNOWN,
	LINUX,
	MAC,
	WINDOWS
};

std::string getOsTypeString()
{
	// WARNING: Don't change these string. The server API relies on them.
	if constexpr (Platform::isWindows())
		return "windows";
	else if constexpr (Platform::isMac())
		return "macOS";
	else if constexpr (Platform::isLinux())
		return "linux";
	else
		return "unknown";
}
*/
